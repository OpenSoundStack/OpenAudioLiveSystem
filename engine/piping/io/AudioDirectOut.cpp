// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

//
// Created by mathis on 13/06/25.
//

#include "AudioDirectOut.h"

AudioDirectOut::AudioDirectOut(AudioRouter *router) : AudioPipe() {
    m_router = router;
}

void AudioDirectOut::feed_packet(AudioPacket &pck) {
    m_router->send_audio_packet(pck, 1);
}
