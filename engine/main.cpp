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

    AudioRouter router{"virbr0", 100, nman.get_net_mapper()};

    nman.init_netman();
    register_pipes(&plumber);

    router.set_routing_callback([&audio_engine](AudioPacket& pck) {
        audio_engine.feed_pipe(pck);
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