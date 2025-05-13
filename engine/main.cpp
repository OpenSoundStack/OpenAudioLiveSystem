#include <iostream>

#include "AudioEngine.h"
#include "log.h"
#include "NetMan.h"

int main() {
    NetMan nman{};
    AudioEngine audio_engine{};

    nman.init_netman();

    if (audio_engine.init_engine() != INIT_OK) {
        std::cerr << LOG_PREFIX << " Failed to initialize audio engine..." << std::endl;
        return -1;
    }

    std::cout << "Initialized Audio Engine." << std::endl;
    std::cout << AUDIO_ENGINE_MAX_PIPES << " pipes available." << std::endl;

    while (true) {
        audio_engine.update_pipes();
        nman.update_netman();
    }

    return 0;
}