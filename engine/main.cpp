// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#include <iostream>
#include <thread>
#include <filesystem>
#include <atomic>

#include "AudioEngine.h"
#include "log.h"
#include "NetMan.h"

#include "piping/AudioPlumber.h"

#include "modules.h"
#include "routing_routines.h"

#include "OpenAudioNetwork/common/AudioRouter.h"
#include "OpenAudioNetwork/common/ClockMaster.h"

#include "linux/sched.h"

void set_thread_realtime(uint8_t prio) {
    sched_param sparams{};
    sparams.sched_priority = prio;

    if (sched_setscheduler(0, SCHED_FIFO, &sparams) == -1) {
        std::cerr << "Failed to set thread realtime..." << std::endl;
    }
}

void set_running_cpu(int cpu_id) {
    cpu_set_t cs{};
    CPU_ZERO(&cs);
    CPU_SET(cpu_id, &cs);

    if (sched_setaffinity(0, sizeof(cpu_set_t), &cs) != 0) {
        std::cerr << "Failed to set affinity..." << std::endl;
    }
}

int main(int argc, char* argv[]) {

    /*
     * Param structure : ./oalsengine eth_iface
     */

    std::string eth_interface = "lo";
    if (argc > 1) {
        eth_interface = std::string(argv[1]);
    }

    // This flag serve to synchronize a halt of the audio processing
    // and a critical change on the audio pipeline. e.g. an engine reset.
    // If set by another thread, the flag makes the processing halt. Then
    // the processing thread clears it and waits for it to be set by the
    // other thread to finish the transaction.
    //
    // WARNING: THE FLAG MUST NOT BE SET WHILE A TRANSACTION IS GOING ON!
    std::atomic_flag disable_processing{};
    disable_processing.clear();

    AudioPlumber plumber{};
    AudioEngine audio_engine{};
    NetMan nman{&plumber};

    AudioRouter router{100};

    if (!nman.init_netman(eth_interface)) {
        std::cerr << LOG_PREFIX << "Failed to initialize network manager." << std::endl;
    }

    if (!router.init_router(eth_interface, nman.get_net_mapper())) {
        std::cerr << LOG_PREFIX << "Failed to initialize audio router." << std::endl;
        exit(-2);
    }

    register_pipes(&plumber, &router, nman.get_net_mapper());

    router.set_routing_callback([&audio_engine](AudioPacket& pck, LowLatHeader& llhdr) {
        audio_engine.feed_pipe(pck);
    });

    router.set_control_callback([&audio_engine](ControlPacket& pck, LowLatHeader& llhdr) {
        audio_engine.queue_control_packet(pck);
    });

    router.set_pipe_create_callback([&audio_engine, &plumber, &router, &nman](ControlPipeCreatePacket& pck, LowLatHeader& llhdr) {
        std::cout << "Pipe to create " << (int)pck.packet_data.seq << "/" << (int)pck.packet_data.seq_max
                  << " @ channel " << (int)pck.packet_data.channel << " of type " << pck.packet_data.elem_type << std::endl;

        if (control_pipe_create_routing(audio_engine, plumber, router, pck, llhdr)) {
            auto local_res_mapping = nman.get_self_topo();
            local_res_mapping.pipe_resmap = audio_engine.get_channel_usage_map();
            nman.update_self_topo(local_res_mapping);
        }
    });

    router.set_control_query_callback([&audio_engine, &router, &disable_processing](ControlQueryPacket& pck, LowLatHeader& llhdr) {
        switch(pck.packet_data.qtype) {
            case ControlQueryType::PIPE_ALLOC_RESET:
                // Set the processing halt flag and wait for the processing
                // to actually be halted
                disable_processing.test_and_set();
                disable_processing.wait(true);

                reset_dsp_alloc(audio_engine, router, llhdr);

                // Indicate to the processing thread that the reset has finished
                // and that the processing can continue
                disable_processing.test_and_set();
                disable_processing.notify_one();
                break;
            default:
                break;
        }
    });

    if (audio_engine.init_engine() != INIT_OK) {
        std::cerr << LOG_PREFIX << " Failed to initialize audio engine..." << std::endl;
        return -1;
    }

    std::cout << "Initialized Audio Engine. Using interface " << eth_interface << std::endl;
    std::cout << AUDIO_ENGINE_MAX_PIPES << " pipes available." << std::endl;

    auto local_res_mapping = nman.get_self_topo();
    local_res_mapping.phy_out_resmap = 0;
    local_res_mapping.pipe_resmap = audio_engine.get_channel_usage_map();

    nman.update_self_topo(local_res_mapping);

    // Start mapping only when we're sure about our topology
    // thus avoiding sending first a mapping packet with no
    // pipe allocation possible as pipe sync is triggered mostly
    // by mapping events.
    nman.start_mapping();

    auto ploader = std::make_shared<PluginLoader>();
    load_plugins(ploader, &plumber, &router, nman.get_net_mapper());

    std::thread audiopoll_thread = std::thread([&router]() {
        set_thread_realtime(25);
        set_running_cpu(0);

        while (true) {
            router.poll_audio_data(false);
        }
    });

    std::thread controlpoll_thread = std::thread([&router]() {
        set_thread_realtime(20);
        set_running_cpu(1);

        while (true) {
            router.poll_control_packets(false);
        }
    });

    std::thread pipe_updater = std::thread([&audio_engine, &router, &disable_processing]() {
        set_thread_realtime(80);
        set_running_cpu(2);

        timespec thread_wait_time{};
        thread_wait_time.tv_sec = 0;
        thread_wait_time.tv_nsec = 100;

        while (true) {
            if (disable_processing.test()) {
                // Clear the flag to indicate that the processing is halted
                disable_processing.clear();
                disable_processing.notify_one();

                // Wait for the transaction with the other thread to finish
                disable_processing.wait(false);
                disable_processing.clear();
            } else {
                router.poll_local_audio_buffer();
                audio_engine.update_processes();

                // Ensure that control packets are applied to pipes when not processing
                // anything. Pipes are not thread safe, control cannot be applied
                // concurrently with audio processing, espacially if the change involves
                // memory allocation or a change in a container size.
                audio_engine.apply_control_packets();
            }

            // This process is a high-priority realtime process
            // It is a blocking task, to let the other threads run
            // I must add a small wait here
            clock_nanosleep(CLOCK_MONOTONIC, 0, &thread_wait_time, nullptr);
        }
    });

    std::thread clock_syncer = std::thread([&nman]() {
        set_thread_realtime(30);
        set_running_cpu(3);

        timespec thread_wait_time{};
        thread_wait_time.tv_sec = 0;
        thread_wait_time.tv_nsec = 100000;

        while (true) {
            nman.clock_master_process();
            clock_nanosleep(CLOCK_MONOTONIC, 0, &thread_wait_time, nullptr);
        }
    });

    audiopoll_thread.detach();
    controlpoll_thread.detach();
    pipe_updater.detach();
    clock_syncer.detach();

    while (true) {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(500ms);
    }

    return 0;
}