// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#include <iostream>
#include <thread>
#include <filesystem>
#include <cstdlib>
#include <string>

#ifdef OAN_HOST_BACKENDS
#include <unistd.h>   // pause()
#endif

#include "AudioEngine.h"
#include "log.h"
#include "NetMan.h"

#include "piping/AudioPlumber.h"

#include "modules.h"
#include "routing_routines.h"

#include "OpenAudioNetwork/common/AudioRouter.h"
#include "OpenAudioNetwork/common/ClockMaster.h"

#ifdef OAN_UID_AUTOCONF
#include "OpenAudioNetwork/common/UidStore.h"
#endif

#include "OpenAudioNetwork/netutils/platform/rt.h"

#ifdef OAN_UID_AUTOCONF
namespace {

std::string sanitise_iface(const std::string& iface) {
    std::string out;
    out.reserve(iface.size());
    for (char c : iface) {
        out.push_back((std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_') ? c : '_');
    }
    return out;
}

std::string engine_uid_path(const std::string& iface) {
#ifdef OAN_HOST_BACKENDS
    const char* home = std::getenv("HOME");
    std::string base = (home && *home) ? std::string(home) + "/.local/state/oals" : "/tmp/oals";
#else
    std::string base = "/var/lib/oals";
#endif
    return base + "/engine-" + sanitise_iface(iface) + ".uid";
}

} // namespace
#endif

static void print_usage() {
    std::cout <<
        "OALSEngine — Open Audio Live System DSP engine\n"
        "\n"
        "Usage: OALSEngine <iface>\n"
        "\n"
        "  <iface>   Network interface or transport spec to bind on.\n"
        "            Linux production: an L2 interface name (e.g. eth0, lo).\n"
        "            Host dev (macOS / Linux with OAN_HOST_BACKENDS=ON):\n"
        "              sim:<daemon-name>        connect to sim_switch via\n"
        "                                       /tmp/osst-sim-<name>.sock\n"
        "              sim:<name>,mac=02:..:01  override the locally-administered MAC\n"
        "              raw:<ifname>             (Mac BPF — not yet implemented)\n"
        "            Defaults to \"lo\" when omitted.\n"
        "\n"
        "  --renumber  Clear persisted UID before boot, then autoconfigure\n"
        "              from scratch. Useful after a hardware swap.\n"
        "  --help      Show this message.\n"
        "\n"
        "The engine runs four detached RT threads (audio recv, control recv,\n"
        "pipe updater, clock syncer) plus a main thread that parks. It must\n"
        "be co-located with at least one peer (UI / IO board / io_simulator)\n"
        "on the same L2 segment.\n";
}

int main(int argc, char* argv[]) {
    std::string eth_interface = "lo";
    bool renumber = false;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            print_usage();
            return 0;
        }
        if (arg == "--renumber") {
            renumber = true;
            continue;
        }
        eth_interface = std::move(arg);
    }

    AudioPlumber plumber{};
    AudioEngine audio_engine{};
    NetMan nman{&plumber};

#ifdef OAN_UID_AUTOCONF
    auto file_store = std::make_unique<FileUidStore>(engine_uid_path(eth_interface));
    if (renumber) {
        std::cout << LOG_PREFIX << "--renumber: clearing persisted UID at "
                  << engine_uid_path(eth_interface) << std::endl;
        file_store->clear();
    }
    EnvOverrideUidStore uid_store{"OAN_PERSISTED_UID", std::move(file_store)};

    if (!nman.init_netman(eth_interface, &uid_store)) {
        std::cerr << LOG_PREFIX << "Failed to initialize network manager." << std::endl;
        return -1;
    }
#else
    (void)renumber;
    if (!nman.init_netman(eth_interface)) {
        std::cerr << LOG_PREFIX << "Failed to initialize network manager." << std::endl;
    }
#endif

    AudioRouter router{nman.committed_uid()};

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

    // Start mapping only when we're sure about our topology
    // thus avoiding sending first a mapping packet with no
    // pipe allocation possible as pipe sync is triggered mostly
    // by mapping events.
    nman.start_mapping();

    auto ploader = std::make_shared<PluginLoader>();
    load_plugins(ploader, &plumber, &router, nman.get_net_mapper());

    std::thread audiopoll_thread = std::thread([&router]() {
        oals::rt::set_thread_realtime(25);
        oals::rt::set_running_cpu(1);

        while (true) {
            router.poll_audio_data(false);
        }
    });

    std::thread controlpoll_thread = std::thread([&router]() {
        oals::rt::set_thread_realtime(20);
        oals::rt::set_running_cpu(1);

        while (true) {
            router.poll_control_packets(false);
        }
    });

    std::thread pipe_updater = std::thread([&audio_engine, &router]() {
        oals::rt::set_thread_realtime(80);
        oals::rt::set_running_cpu(2);

        while (true) {
#ifdef OAN_HOST_BACKENDS
            // Block until the audio recv callback signals a new block, or
            // until the heartbeat timeout (lets continuous_process keep
            // running time-driven work — release envelopes etc. — when
            // the wire is idle). 1 ms is well below the 667 µs block
            // period at 96 kHz so it can never delay a real block.
            audio_engine.wait_for_block(1000);
            router.poll_local_audio_buffer();
            audio_engine.update_processes();
#else
            router.poll_local_audio_buffer();
            audio_engine.update_processes();

            // This process is a high-priority realtime process
            // It is a blocking task, to let the other threads run
            // I must add a small wait here
            oals::rt::precise_sleep_ns(100);
#endif
        }
    });

    std::thread clock_syncer = std::thread([&nman]() {
        oals::rt::set_running_cpu(3);

        while (true) {
#ifdef OAN_HOST_BACKENDS
            // Block in poll() up to 200 ms for sync recv. clock_master_process
            // self-paces its 1 s broadcast heartbeat internally, so the
            // 200 ms wake cadence is more than enough to keep it firing.
            nman.clock_wait_or_tick(200);
#else
            nman.clock_master_process();
            oals::rt::precise_sleep_ns(10000);
#endif
        }
    });

    audiopoll_thread.detach();
    controlpoll_thread.detach();
    pipe_updater.detach();
    clock_syncer.detach();

#ifdef OAN_HOST_BACKENDS
    // The detached worker threads do all the real work; main has nothing
    // to do but stay alive until SIGTERM/SIGINT. update_netman() is empty
    // today, so the Linux while-loop below is a CPU-melting no-op on Mac.
    // Park on pause() instead — Ctrl-C / kill still ends the process.
    while (true) {
        pause();
    }
#else
    while (true) {
        nman.update_netman();
    }
#endif

    return 0;
}