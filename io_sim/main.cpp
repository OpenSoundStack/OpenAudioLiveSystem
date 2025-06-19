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
#include <memory>
#include <cmath>
#include <chrono>
#include <queue>

#include <alsa/asoundlib.h>

#include <OpenAudioNetwork/common/NetworkMapper.h>
#include <OpenAudioNetwork/common/packet_structs.h>

#include <linux/sched.h>

float sig_gen(float f, float gain, int n) {
    constexpr float T = 1.0f / 96000.0f;
    float pulse = 2.0f * 3.141592 * f;
    float pulse_10 = pulse / 1000.0f;

    float sample = (float)sin(pulse * n * T) * (float)(sin(1.0f * n * T) * 0.5f + 1.0f) * gain;

    return sample;
}

uint64_t local_now_us() {
    return std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()
    ).count();
}

AudioPacket make_packet(float f, float sig_level, int& n) {
    AudioPacket pck{};
    pck.header.type = PacketType::AUDIO;
    pck.packet_data.channel = 0;

    for (int i = 0; i < AUDIO_DATA_SAMPLES_PER_PACKETS; i++) {
        pck.packet_data.samples[i] = sig_gen(f, sig_level, n);
        n++;
    }

    return pck;
}

snd_pcm_t* alsa_setup() {
    snd_pcm_t* hdl;
    snd_pcm_hw_params_t* params;
    snd_pcm_sw_params_t* sw_params;
    snd_pcm_format_t fmt = SND_PCM_FORMAT_FLOAT;

    auto err = snd_pcm_open(&hdl, "default", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
    if (err < 0) {
        std::cerr << "ALSA FAIL INIT" << std::endl;
    }

    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(hdl, params);
    snd_pcm_hw_params_set_access(hdl, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(hdl, params, SND_PCM_FORMAT_FLOAT);
    snd_pcm_hw_params_set_channels(hdl, params, 1);
    snd_pcm_hw_params_set_rate(hdl, params, 96000, 0);

    if (snd_pcm_hw_params(hdl, params) < 0) {
        std::cerr << "FAILED TO SET HW PARAMS" << std::endl;
    }

    snd_pcm_sw_params_malloc(&sw_params);
    snd_pcm_sw_params_current(hdl, sw_params);
    snd_pcm_sw_params_set_start_threshold(hdl, sw_params, AUDIO_DATA_SAMPLES_PER_PACKETS * 1000);

    err = snd_pcm_sw_params(hdl, sw_params);
    if (err < 0) {
        std::cerr << "FAILED TO SET SW PARAMS : " << err << std::endl;
    }

    snd_pcm_prepare(hdl);
    return hdl;
}

int main(int argc, char* argv[]) {
    std::cout << "OpenAudioLive IO Emulator" << std::endl;

    PeerConf conf{};
    conf.iface = "virbr0";

    const char name[32] = "IOSIM";
    memcpy(&conf.dev_name, name, strlen(name));

    conf.sample_rate = SamplingRate::SAMPLING_96K;
    conf.dev_type = DeviceType::AUDIO_IO_INTERFACE;
    conf.uid = 1;
    conf.topo.phy_in_count = 4;
    conf.topo.phy_out_count = 4;
    conf.topo.pipes_count = 1;

    snd_pcm_t* sound_handle = alsa_setup();

    // Init auto-discover mechanism
    std::shared_ptr<NetworkMapper> nmapper = std::make_shared<NetworkMapper>(conf);

    std::cout << "Initializing on " << conf.iface << std::endl;
    if(nmapper->init_mapper(conf.iface)) {
        nmapper->launch_mapping_process();
    } else {
        std::cerr << "Failed to init mapper" << std::endl;
        exit(-1);
    }

    LowLatSocket audio_iface(conf.uid, nmapper);
    audio_iface.init_socket(conf.iface, EthProtocol::ETH_PROTO_OANAUDIO);

    LowLatSocket control_iface(conf.uid, nmapper);
    control_iface.init_socket(conf.iface, EthProtocol::ETH_PROTO_OANCONTROL);

    std::array<int, 16> ncounters;

    auto last_now = local_now_us();

    std::thread playback_thread = std::thread([&audio_iface, sound_handle]() {
        constexpr int buf_size_mult = 100;
        std::queue<AudioData> audio_buffer;
        int buffer_cursor = 0;
        LowLatPacket<AudioPacket> rx_packet{};

        while (true) {
            if (audio_iface.receive_data(&rx_packet) > 0) {
                auto pck_data = rx_packet.payload.packet_data;
                audio_buffer.emplace(pck_data);
            }

            if (!audio_buffer.empty()) {
                auto& oldest_pck = audio_buffer.front();

                int err = snd_pcm_writei(sound_handle, &oldest_pck.samples, AUDIO_DATA_SAMPLES_PER_PACKETS);
                if (err < 0) {
                    //std::cerr << "FAIL : " << err << std::endl;
                    snd_pcm_prepare(sound_handle);
                }

                audio_buffer.pop();
            }
       }
    });

    playback_thread.detach();

    sched_param params{};
    params.sched_priority = 10;
    if (sched_setscheduler(0, SCHED_RR, &params) != 0) {
        std::cerr << "FAILED TO SET SCHED" << std::endl;
    }

    timespec ts{};
    ts.tv_sec = 0;
    ts.tv_nsec = (long)((AUDIO_DATA_SAMPLES_PER_PACKETS * (1.0f / 96000.0f)) * 1e9);

    auto last_stamp = local_now_us();

    while (true) {
        for (int i = 0; i < 1; i++) {
            AudioPacket packet = make_packet((i + 1) * 300, ((float)i + 1)  / 4.0f, ncounters[i]);
            packet.header.timestamp = local_now_us();
            packet.packet_data.channel = i;

            audio_iface.send_data(packet, 100);
        }

        clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, nullptr);
    }

    return 0;
}