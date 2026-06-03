// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#include <iostream>
#include <memory>
#include <cmath>
#include <chrono>
#include <queue>

#include <sndfile.h>

#include <OpenAudioNetwork/common/NetworkMapper.h>
#include <OpenAudioNetwork/common/packet_structs.h>
#include <OpenAudioNetwork/common/ClockSlave.h>

#include <OpenAudioNetwork/netutils/platform/rt.h>

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
    conf.iface = "virbr0";
    //conf.iface = "enx9cbf0d008387";

    const char name[32] = "IOSIM";
    memcpy(&conf.dev_name, name, strlen(name));

    conf.sample_rate = SamplingRate::SAMPLING_96K;
    conf.dev_type = DeviceType::AUDIO_IO_INTERFACE;
    conf.uid = 1;
    conf.topo.phy_in_count = 4;
    conf.topo.phy_out_count = 4;
    conf.topo.pipes_count = 1;
    conf.ck_type = CKTYPE_SLAVE;

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

    oals::rt::set_process_scheduler_rr(99);

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

    oals::rt::set_thread_realtime(50);

    std::cout << "START" << std::endl;

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

        oals::rt::precise_sleep_ns(ts.tv_nsec);
        ts.tv_nsec = wait_base;
    }

    return 0;
}