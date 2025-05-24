#include <iostream>

#include "AudioEngine.h"
#include "log.h"
#include "NetMan.h"
#include "piping/AudioPlumber.h"

#include "OpenAudioNetwork/common/base_pipes/AudioInPipe.h"
#include "engine/piping/feedback/LevelMeasurePipe.h"
#include "piping/filtering/FiltHPFPipe.h"
#include "piping/filtering/FiltLPFPipe.h"
#include "routing_routines.h"

#include "OpenAudioNetwork/common/AudioRouter.h"

void register_pipes(AudioPlumber* plumber, AudioRouter* router) {
    plumber->register_pipe_element("audioin", []() {
        return std::make_shared<AudioInPipe>();
    });

    plumber->register_pipe_element("dbmeas", [router]() {
        return std::make_shared<LevelMeasurePipe>(router);
    });

    plumber->register_pipe_element("hpf1", []() {
        return std::make_shared<FiltHPFPipe>();
    });

    plumber->register_pipe_element("lpf1", [](){
        return std::make_shared<FiltLPFPipe>();
    });
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

    register_pipes(&plumber, &router);

    router.set_routing_callback([&audio_engine](AudioPacket& pck, LowLatHeader& llhdr) {
        audio_engine.feed_pipe(pck);
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

    while (true) {
        router.poll_audio_data();
        router.poll_control_packets();
        nman.update_netman();
    }

    return 0;
}