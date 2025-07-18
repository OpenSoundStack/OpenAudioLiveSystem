// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2025 - Mathis DELGADO
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, version 3 of the License.
//
// This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.

#include <iostream>
#include <thread>
#include <filesystem>

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
        audio_engine.propagate_control(pck);
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

    router.set_control_query_callback([&audio_engine, &router](ControlQueryPacket& pck, LowLatHeader& llhdr) {
        switch(pck.packet_data.qtype) {
            case ControlQueryType::PIPE_ALLOC_RESET:
                reset_dsp_alloc(audio_engine, router, llhdr);
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

    auto ploader = std::make_shared<PluginLoader>();
    load_plugins(ploader, &plumber, &router, nman.get_net_mapper());

    auto local_res_mapping = nman.get_self_topo();
    local_res_mapping.phy_out_resmap = 0;
    local_res_mapping.pipe_resmap = audio_engine.get_channel_usage_map();

    nman.update_self_topo(local_res_mapping);

    std::thread audiopoll_thread = std::thread([&router]() {
        set_thread_realtime(25);
        set_running_cpu(1);

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

    std::thread pipe_updater = std::thread([&audio_engine, &router]() {
        set_thread_realtime(80);
        set_running_cpu(2);

        timespec thread_wait_time{};
        thread_wait_time.tv_sec = 0;
        thread_wait_time.tv_nsec = 100;

        while (true) {
            router.poll_local_audio_buffer();
            audio_engine.update_processes();

            // This process is a high-priority realtime process
            // It is a blocking task, to let the other threads run
            // I must add a small wait here
            clock_nanosleep(CLOCK_MONOTONIC, 0, &thread_wait_time, nullptr);
        }
    });

    std::thread clock_syncer = std::thread([&nman]() {
        set_running_cpu(3);

        timespec thread_wait_time{};
    thread_wait_time.tv_sec = 0;
    thread_wait_time.tv_nsec = 10000;

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
        nman.update_netman();
    }

    return 0;
}