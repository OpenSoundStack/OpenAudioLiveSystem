#include <iostream>

#include "AudioEngine.h"
#include "NetMan.h"

int main() {
    NetMan nman{};
    AudioEngine audio_engine{};

    nman.init_netman();

    if (audio_engine.init_engine() != INIT_OK) {
        std::cerr << __FILE_NAME__ << ", line " << __LINE__ << ", error : Failed to initialize audio engine..." << std::endl;
        return -1;
    }

    std::cout << "Initialized Audio Engine." << std::endl;
    std::cout << AUDIO_ENGINE_MAX_PIPES << " pipes available." << std::endl;

    while (true) {
        audio_engine.update_pipes();
    }

    return 0;
}