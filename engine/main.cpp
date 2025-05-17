#include <iostream>

#include "AudioEngine.h"
#include "log.h"
#include "NetMan.h"
#include "piping/AudioPlumber.h"

#include "OpenAudioNetwork/common/base_pipes/AudioInPipe.h"
#include "OpenAudioNetwork/common/base_pipes/LevelMeasurePipe.h"
#include "piping/filtering/FiltHPFPipe.h"
#include "piping/filtering/FiltLPFPipe.h"

#include "OpenAudioNetwork/common/AudioRouter.h"

void register_pipes(AudioPlumber* plumber) {
    plumber->register_pipe_element("audioin", []() {
        return std::make_shared<AudioInPipe>();
    });

    plumber->register_pipe_element("dbmeas", []() {
        return std::make_shared<LevelMeasurePipe>();
    });

    plumber->register_pipe_element("hpf1", []() {
        return std::make_shared<FiltHPFPipe>();
    });

    plumber->register_pipe_element("lpf1", [](){
        return std::make_shared<FiltLPFPipe>();
    });
}

int main() {
    AudioPlumber plumber{};
    AudioEngine audio_engine{};
    NetMan nman{&plumber};

    AudioRouter router{100};

    if (!nman.init_netman()) {
        std::cerr << LOG_PREFIX << "Failed to initialize network manager." << std::endl;
    }

    register_pipes(&plumber);

    if (!router.init_router("virbr0", nman.get_net_mapper())) {
        std::cerr << LOG_PREFIX << "Failed to initialize audio router." << std::endl;
        exit(-2);
    }

    router.set_routing_callback([&audio_engine](AudioPacket& pck, LowLatHeader& llhdr) {
        audio_engine.feed_pipe(pck);
    });

    router.set_pipe_create_callback([&audio_engine, &plumber, &router](ControlPipeCreatePacket& pck, LowLatHeader& llhdr) {
        std::cout << "Pipe to create " << (int)pck.packet_data.seq << "/" << (int)pck.packet_data.seq_max
                  << " @ channel " << (int)pck.packet_data.channel << " of type " << pck.packet_data.elem_type << std::endl;

        ControlResponsePacket response;
        response.header.timestamp = pck.header.timestamp; // Use the initial packet stamp as identifier
        response.header.type = PacketType::CONTROL_RESPONSE;
        response.packet_data.response = ControlResponseCode::CREATE_OK;
        response.packet_data.seq = 1;

        router.send_control_packet_response(response, llhdr.sender_uid);
    });

    if (audio_engine.init_engine() != INIT_OK) {
        std::cerr << LOG_PREFIX << " Failed to initialize audio engine..." << std::endl;
        return -1;
    }

    std::cout << "Initialized Audio Engine." << std::endl;
    std::cout << AUDIO_ENGINE_MAX_PIPES << " pipes available." << std::endl;

    while (true) {
        router.poll_audio_data();
        router.poll_control_packets();
        nman.update_netman();
    }

    return 0;
}