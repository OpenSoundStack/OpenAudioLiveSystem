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
#include <semaphore>

#include "AudioEngine.h"
#include "log.h"
#include "NetMan.h"

#include "piping/AudioPlumber.h"
#include "piping/io/AudioInPipe.h"
#include "piping/feedback/LevelMeasurePipe.h"
#include "piping/filtering/FiltHPFPipe.h"
#include "piping/filtering/FiltLPFPipe.h"
#include "piping/io/AudioSendMtx.h"
#include "piping/io/AudioInMtx.h"
#include "piping/io/AudioDirectOut.h"

#include "routing_routines.h"

#include "OpenAudioNetwork/common/AudioRouter.h"
#include "OpenAudioNetwork/common/ClockMaster.h"

#include "linux/sched.h"
#include "sys/syscall.h"
#include "unistd.h"
#include "asm-generic/unistd.h"
#include "linux/sched/types.h"

void register_pipes(AudioPlumber* plumber, AudioRouter* router, std::shared_ptr<NetworkMapper> nmapper) {
    plumber->register_pipe_element("audioin", []() {
        return std::make_shared<AudioInPipe>();
    });

    plumber->register_pipe_element("dbmeas", [router, nmapper]() {
        return std::make_shared<LevelMeasurePipe>(router, nmapper);
    });

    plumber->register_pipe_element("hpf1", []() {
        return std::make_shared<FiltHPFPipe>();
    });

    plumber->register_pipe_element("lpf1", [](){
        return std::make_shared<FiltLPFPipe>();
    });

    plumber->register_pipe_element("sendmtx", [router]() {
        return std::make_shared<AudioSendMtx>(router);
    });

    plumber->register_pipe_element("inmtx", []() {
        return std::make_shared<AudioInMtx>();
    });

    plumber->register_pipe_element("dirout", [router]() {
        return std::make_shared<AudioDirectOut>(router);
    });
}

void set_thread_realtime(uint8_t prio) {
    sched_param sparams{};
    sparams.sched_priority = prio;

    if (sched_setscheduler(0, SCHED_FIFO, &sparams) != 0) {
        std::cerr << "Failed to set thread realtime..." << std::endl;
    }
}

/*
int sched_setattr(pid_t pid, struct sched_attr* attr, unsigned int flags) {
    return syscall(__NR_sched_setattr, pid, attr, flags);
}

void set_thread_deadline(uint64_t max_time_ns) {
    sched_attr sattr{};
    sattr.sched_policy = SCHED_DEADLINE;
    sattr.sched_flags = 0;
    sattr.sched_nice = 0;
    sattr.sched_priority = 0;
    sattr.sched_period = max_time_ns;
    sattr.sched_deadline = max_time_ns;
    sattr.sched_runtime = max_time_ns;
    sattr.size = sizeof(sattr);

    if (sched_setattr(0, &sattr, 0) != 0) {
        std::cerr << "Failed to enable deadline sched for this thread. Error is " << errno << std::endl;
    }
}
*/

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

    auto local_res_mapping = nman.get_self_topo();
    local_res_mapping.phy_out_resmap = 0;
    local_res_mapping.pipe_resmap = audio_engine.get_channel_usage_map();

    nman.update_self_topo(local_res_mapping);

    std::thread audiopoll_thread = std::thread([&router]() {
        set_thread_realtime(25);

        while (true) {
            router.poll_audio_data();
        }
    });

    std::thread controlpoll_thread = std::thread([&router]() {
        set_thread_realtime(20);

        while (true) {
            router.poll_control_packets(false);
        }
    });

    std::thread pipe_updater = std::thread([&audio_engine, &router]() {
        // I am expecting maximum 1 ms latency for each pipe
        constexpr uint64_t max_deadline = (1000000 * AUDIO_ENGINE_MAX_PIPES) / 96000;
        //set_thread_deadline(max_deadline);

        set_thread_realtime(50);

        while (true) {
            router.poll_local_audio_buffer();
            audio_engine.update_processes();
        }
    });

    std::thread clock_syncer = std::thread([&nman]() {
        while (true) {
            nman.clock_master_process();
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