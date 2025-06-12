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

#include <OpenAudioNetwork/common/NetworkMapper.h>
#include <OpenAudioNetwork/common/packet_structs.h>

float sig_gen(float f, int sig_level, int n) {
    constexpr float T = 1.0f / 96000.0f;
    float pulse = 2.0f * 3.141592 * f;
    float pulse_10 = pulse / 1000.0f;

    float sample = (float)sin(pulse * n * T) * (float)(sin(pulse_10 * n * T) * 0.5f + 1.0f) * (float)(1 << sig_level);

    return sample / (1 << 24);
}

uint64_t local_now_us() {
    return std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()
    ).count();
}

AudioPacket make_packet(int sig_level, int& n) {
    AudioPacket pck{};
    pck.header.type = PacketType::AUDIO;
    pck.packet_data.channel = 0;

    for (int i = 0; i < 64; i++) {
        pck.packet_data.samples[i] = sig_gen(1000.0f, sig_level, n);
        n++;
    }

    return pck;
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
    conf.topo.phy_in_count = 1;
    conf.topo.phy_out_count = 0;
    conf.topo.pipes_count = 1;

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
    while (true) {
        auto now = local_now_us();

        auto delta = now - last_now;
        if (delta >= (64.0f / 0.096f)) {
            for (int i = 0; i < 1; i++) {
                AudioPacket packet = make_packet(24 - i, ncounters[i]);
                packet.packet_data.channel = i;

                audio_iface.send_data(packet, 100);
            }

            last_now = local_now_us();
        }
    }

    return 0;
}