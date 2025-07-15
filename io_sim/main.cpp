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
#include <sndfile.h>

#include <OpenAudioNetwork/common/NetworkMapper.h>
#include <OpenAudioNetwork/common/packet_structs.h>
#include <OpenAudioNetwork/common/ClockSlave.h>

#include <linux/sched.h>

void set_thread_realtime(uint8_t prio) {
    sched_param sparams{};
    sparams.sched_priority = prio;

    if (sched_setscheduler(0, SCHED_FIFO, &sparams) != 0) {
        std::cerr << "Failed to set thread realtime..." << std::endl;
    }
}

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

uint64_t local_now_ns() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
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
    snd_pcm_sw_params_set_start_threshold(hdl, sw_params, AUDIO_DATA_SAMPLES_PER_PACKETS * 18);

    err = snd_pcm_sw_params(hdl, sw_params);
    if (err < 0) {
        std::cerr << "FAILED TO SET SW PARAMS : " << err << std::endl;
    }

    snd_pcm_prepare(hdl);
    return hdl;
}

std::vector<AudioPacket> gen_packet_strm_from_file(std::string file, int channel) {
    SF_INFO info{};
    SNDFILE* wavfile = sf_open(file.c_str(), SFM_READ, &info);
    if (!wavfile) {
        std::cerr << "Failed to read " << file << std::endl;
        return {};
    }

    float pending_sample = 0.0f;
    std::vector<AudioPacket> stream_packets;

    AudioPacket pending_packet{};
    pending_packet.header.type = PacketType::AUDIO;
    pending_packet.packet_data.channel = channel;

    int counter = 0;
    while (sf_read_float(wavfile, &pending_sample, 1) != 0) {
        pending_packet.packet_data.samples[counter] = pending_sample;

        counter++;
        if (counter == AUDIO_DATA_SAMPLES_PER_PACKETS) {
            counter = 0;

            stream_packets.push_back(pending_packet);
        }
    }

    sf_close(wavfile);

    return stream_packets;
}

int main(int argc, char* argv[]) {
    std::cout << "OpenAudioLive IO Emulator" << std::endl;

    PeerConf conf{};
    //conf.iface = "virbr0";
    conf.iface = "enx9cbf0d008387";

    const char name[32] = "IOSIM";
    memcpy(&conf.dev_name, name, strlen(name));

    conf.sample_rate = SamplingRate::SAMPLING_96K;
    conf.dev_type = DeviceType::AUDIO_IO_INTERFACE;
    conf.uid = 1;
    conf.topo.phy_in_count = 4;
    conf.topo.phy_out_count = 4;
    conf.topo.pipes_count = 1;
    conf.ck_type = CKTYPE_SLAVE;

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

    ClockSlave cs{1, conf.iface, nmapper};

    std::thread playback_thread = std::thread([&audio_iface, sound_handle, &cs]() {
        std::queue<AudioData> audio_buffer;
        LowLatPacket<AudioPacket> rx_packet{};

        auto last_now = local_now_us();
        uint64_t delay_sum = 0;
        uint64_t processing_latency_sum = 0;
        uint64_t sum_count = 0;
        uint64_t max_delay = 0;
        uint64_t min_delay = 0xFFFFFFFFFFFFFFFF;

        while (true) {
            if (audio_iface.receive_data(&rx_packet) > 0) {
                auto pck_data = rx_packet.payload.packet_data;
                audio_buffer.emplace(pck_data);

                auto now = local_now_us();
                last_now = now;

                auto now_corrected = now - cs.get_ck_offset();
                auto latency = rx_packet.payload.header.prev_delay + (now_corrected - rx_packet.payload.header.timestamp);
                delay_sum += latency;
                processing_latency_sum += rx_packet.payload.header.prev_delay;
                sum_count++;

                if (latency > max_delay) {
                    max_delay = latency;
                }

                if (latency < min_delay) {
                    min_delay = latency;
                }
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

            if (sum_count == 3250) {
                float avg_latency = (float)delay_sum / sum_count;
                float avg_proc_latency = (float)processing_latency_sum / sum_count;

                std::cout << "Avg roudtrip latency " << avg_latency / 1000.0f << " ms";
                std::cout << ", Avg processing latency " << avg_proc_latency / 1000.0f << " ms" << std::endl;
                std::cout << "Max : " << (float)max_delay / 1000.0f << " ms, Min : " << (float)min_delay / 1000.0f << " ms" << std::endl;

                sum_count = 0;
                delay_sum = 0;
                processing_latency_sum = 0;
                min_delay = 0xFFFFFFFFFFFFFFFF;
                max_delay = 0;
            }
       }
    });

    playback_thread.detach();

    sched_param params{};
    params.sched_priority = 99;
    if (sched_setscheduler(0, SCHED_RR, &params) != 0) {
        std::cerr << "FAILED TO SET SCHED" << std::endl;
    }

    auto wait_base = (long)((AUDIO_DATA_SAMPLES_PER_PACKETS * (1.0f / 96000.0f)) * 1e9);

    timespec ts{};
    ts.tv_sec = 0;
    ts.tv_nsec = wait_base;

    std::thread clock_thread = std::thread([&cs]() {
        while (true) {
            cs.sync_process();
        }
    });
    clock_thread.detach();

    int stream_cursor = 0;

    std::vector<std::string> paths = {
        "/home/mathis/osst/audio_test/enc96/Boucle_GC_12.wav",
        "/home/mathis/osst/audio_test/enc96/Boucle_CC_12.wav",
        "/home/mathis/osst/audio_test/enc96/Boucle_OHL_12.wav",
        "/home/mathis/osst/audio_test/enc96/Boucle_OHR_12.wav",
        "/home/mathis/osst/audio_test/enc96/Boucle_Perc_12.wav",
        "/home/mathis/osst/audio_test/enc96/Boucle_Solo_12.wav",
        "/home/mathis/osst/audio_test/enc96/Boucle_Bois_12 L.wav",
        "/home/mathis/osst/audio_test/enc96/Boucle_Bois_12 R.wav",

    };
    std::vector<std::vector<AudioPacket>> stems_loop;

    int chann = 0;
    for (auto& p : paths) {
        stems_loop.emplace_back(gen_packet_strm_from_file(p, chann));
        chann++;
    }

    set_thread_realtime(50);

    while (true) {
        auto start = local_now_ns();

        auto off = cs.get_ck_offset();

        for (auto& loop : stems_loop) {
            loop[stream_cursor].header.timestamp = NetworkMapper::local_now_us() - off;
            audio_iface.send_data(loop[stream_cursor], 100);
        }

        stream_cursor = (stream_cursor + 1) % stems_loop[0].size();

        auto sent = local_now_ns();
        ts.tv_nsec -= (sent - start);

        //auto now = local_now_us();
        //std::cout << "Delta tx," << now - last_stamp << "," << now << std::endl;

        //last_stamp = now;

        clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, nullptr);
        ts.tv_nsec = wait_base;
    }

    return 0;
}