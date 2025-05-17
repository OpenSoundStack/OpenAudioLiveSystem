#include <iostream>
#include <memory>
#include <cmath>
#include <chrono>

#include <OpenAudioNetwork/common/NetworkMapper.h>
#include <OpenAudioNetwork/common/packet_structs.h>

float sig_gen(float f, int sig_level) {
    constexpr float T = 1.0f / 96000.0f;
    float pulse = 2.0f * 3.141592 * f;

    static int n = 0;


    float sample = (float)sin(pulse * n * T) * (float)(1 << sig_level);

    n++;
    return sample;
}

uint64_t local_now_us() {
    return std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()
    ).count();
}

AudioPacket make_packet() {
    AudioPacket pck{};
    pck.header.type = PacketType::AUDIO;
    pck.packet_data.channel = 1;

    for (int i = 0; i < 64; i++) {
        pck.packet_data.samples[i] = sig_gen(1000.0f, 20);
    }

    return pck;
}

int main(int argc, char* argv[]) {
    std::cout << "OpenAudioLive IO Emulator" << std::endl;

    PeerConf conf{};
    conf.iface = "enp1s0";

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

    char audio_in_char[32] = "audioin";

    ControlPipeCreatePacket pc{};
    pc.header.type = PacketType::CONTROL_CREATE;
    pc.packet_data.channel = 1;
    pc.packet_data.stack_position = 0;
    memcpy(pc.packet_data.elem_type, audio_in_char, 32);

    auto last_now = local_now_us();
    auto last_creation = local_now_us();
    while (true) {
        auto now = local_now_us();

        auto delta = now - last_now;
        if (delta >= (64.0f / 0.096f)) {
            AudioPacket packet = make_packet();
            audio_iface.send_data(packet, 100);

            last_now = local_now_us();
        }

        if (now - last_creation >= 5000000) {
            control_iface.send_data(pc, 100);
            last_creation = local_now_us();
        }
    }

    return 0;
}