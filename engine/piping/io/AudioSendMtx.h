// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef AUDIOOUTMTX_H
#define AUDIOOUTMTX_H

#include "OpenAudioNetwork/common/AudioRouter.h"

#include "plugins/loader/AudioPipe.h"

#include <unordered_map>

struct FaderControlFrame {
    uint8_t channel;
    uint16_t host;
    float level;
};

class AudioSendMtx : public AudioPipe {
public:
    AudioSendMtx(AudioRouter* router);
    ~AudioSendMtx() override = default;

    void feed_packet(AudioPacket &pck) override;

protected:
    float process_sample(float sample) override;
    void apply_control(ControlPacket &pck) override;

private:
    void process_packet(AudioPacket& pck, uint8_t fader_channel);

    AudioRouter* m_router;
    std::unordered_map<uint8_t, FaderControlFrame> m_fader_map;
};



#endif //AUDIOOUTMTX_H
