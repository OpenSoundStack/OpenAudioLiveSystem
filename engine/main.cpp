#include <iostream>

#include "AudioEngine.h"
#include "log.h"
#include "NetMan.h"
#include "piping/AudioPlumber.h"

#include "OpenAudioNetwork/common/base_pipes/AudioInPipe.h"
#include "OpenAudioNetwork/common/base_pipes/LevelMeasurePipe.h"
#include "OpenAudioNetwork/common/AudioRouter.h"

void register_pipes(AudioPlumber* plumber) {
    plumber->register_pipe_element("audioin", []() {
        return std::make_unique<AudioInPipe>();
    });

    plumber->register_pipe_element("dbmeas", []() {
        return std::make_unique<LevelMeasurePipe>();
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

    std::vector<std::string> pipe_blueprint = {"audioin", "dbmeas"};
    auto pipe = plumber.construct_pipe(pipe_blueprint).value();

    audio_engine.install_pipe(1, pipe);

    while (true) {
        router.poll_audio_data();
        nman.update_netman();
    }

    return 0;
}