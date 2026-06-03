// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#include <iostream>
#include <fstream>
#include <memory>
#include <cmath>
#include <chrono>
#include <queue>
#include <string>
#include <vector>
#include <cstdlib>

#include <sndfile.h>
#include <nlohmann/json.hpp>

#include <OpenAudioNetwork/common/NetworkMapper.h>
#include <OpenAudioNetwork/common/packet_structs.h>
#include <OpenAudioNetwork/common/ClockSlave.h>

#include <OpenAudioNetwork/netutils/platform/rt.h>

static constexpr int IO_SIM_SAMPLE_RATE = 96000;

std::string expand_tilde(const std::string& p) {
    if (p.empty() || p[0] != '~') return p;
    const char* home = std::getenv("HOME");
    if (!home) return p;
    return std::string(home) + p.substr(1);
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

std::vector<AudioPacket> gen_packet_strm_from_file(std::string file, int channel) {
    SF_INFO info{};
    SNDFILE* wavfile = sf_open(file.c_str(), SFM_READ, &info);
    if (!wavfile) {
        std::cerr << "io_sim: failed to open " << file << std::endl;
        return {};
    }

    if (info.samplerate != IO_SIM_SAMPLE_RATE) {
        std::cerr << "io_sim: " << file << " is " << info.samplerate
                  << " Hz, io_sim runs at " << IO_SIM_SAMPLE_RATE << " Hz.\n"
                  << "        Convert with: ffmpeg -i \"" << file
                  << "\" -ar " << IO_SIM_SAMPLE_RATE << " \"" << file << ".96k.wav\"\n";
        sf_close(wavfile);
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

// Build a one-second tone loop as a packet stream. Used for "tone" tracks in
// the config — handy when you don't have a wav handy but want something audible.
std::vector<AudioPacket> gen_packet_strm_tone(float freq_hz, float gain, int channel) {
    constexpr int LOOP_SAMPLES = IO_SIM_SAMPLE_RATE;
    const int n_packets = LOOP_SAMPLES / AUDIO_DATA_SAMPLES_PER_PACKETS;

    std::vector<AudioPacket> stream_packets;
    stream_packets.reserve(n_packets);

    const float two_pi_f_over_sr = 2.0f * 3.14159265358979f * freq_hz / (float)IO_SIM_SAMPLE_RATE;

    int n = 0;
    for (int p = 0; p < n_packets; ++p) {
        AudioPacket pkt{};
        pkt.header.type = PacketType::AUDIO;
        pkt.packet_data.channel = channel;
        for (int i = 0; i < AUDIO_DATA_SAMPLES_PER_PACKETS; ++i) {
            pkt.packet_data.samples[i] = std::sin(two_pi_f_over_sr * n) * gain;
            n++;
        }
        stream_packets.push_back(pkt);
    }
    return stream_packets;
}

static void print_usage() {
    std::cout <<
        "io_simulator — looping audio source for the OALS dev stack\n"
        "\n"
        "Usage: io_simulator <iface> [config-path]\n"
        "\n"
        "  <iface>         Network interface or transport spec, same as OALSEngine.\n"
        "                  Linux: an L2 ifname. Host dev: sim:<daemon-name>.\n"
        "                  Defaults to \"virbr0\" when omitted.\n"
        "  [config-path]   Path to the io_sim JSON track config.\n"
        "                  Defaults to ./io_sim.json. Example template in\n"
        "                  io_sim/io_sim.example.json.\n"
        "\n"
        "  --help          Show this message.\n"
        "\n"
        "Loops the configured tracks (tone or .wav stems) onto the OAN audio\n"
        "EtherType at 96 kHz, advertising itself as an AUDIO_IO_INTERFACE\n"
        "with uid from the config (default 1) and acting as a ClockSlave to\n"
        "whatever ClockMaster is on the segment.\n";
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::string a = argv[1];
        if (a == "--help" || a == "-h") {
            print_usage();
            return 0;
        }
    }

    std::cout << "OpenAudioLive IO Emulator" << std::endl;

    const std::string config_path = (argc > 2) ? argv[2] : "io_sim.json";

    nlohmann::json cfg;
    {
        std::ifstream f(config_path);
        if (!f) {
            std::cerr << "io_sim: failed to open config '" << config_path
                      << "'. Pass a path as argv[2] or place io_sim.json in cwd.\n";
            return -1;
        }
        try {
            f >> cfg;
        } catch (const std::exception& e) {
            std::cerr << "io_sim: failed to parse '" << config_path << "': "
                      << e.what() << std::endl;
            return -1;
        }
    }

    PeerConf conf{};
    conf.iface = (argc > 1) ? argv[1] : "virbr0";

    const char name[32] = "IOSIM";
    memcpy(&conf.dev_name, name, strlen(name));

    conf.sample_rate = SamplingRate::SAMPLING_96K;
    conf.dev_type = DeviceType::AUDIO_IO_INTERFACE;
    conf.uid = cfg.value("uid", 1);
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
#ifdef OAN_HOST_BACKENDS
            // Pace the otherwise-tight async recv with a poll() timeout
            // so we don't burn a core spinning on EAGAIN.
            cs.wait_sync_readable(200);
#endif
            cs.sync_process();
        }
    });
    clock_thread.detach();

    int stream_cursor = 0;

    if (!cfg.contains("tracks") || !cfg["tracks"].is_array() || cfg["tracks"].empty()) {
        std::cerr << "io_sim: config '" << config_path
                  << "' has no 'tracks' array (or it's empty)." << std::endl;
        return -1;
    }

    std::vector<std::vector<AudioPacket>> stems_loop;
    stems_loop.reserve(cfg["tracks"].size());

    for (const auto& t : cfg["tracks"]) {
        if (!t.contains("channel")) {
            std::cerr << "io_sim: track entry missing 'channel'" << std::endl;
            return -1;
        }
        int chann = t["channel"].get<int>();

        std::vector<AudioPacket> stream;
        if (t.contains("path")) {
            std::string p = expand_tilde(t["path"].get<std::string>());
            stream = gen_packet_strm_from_file(p, chann);
            if (stream.empty()) return -1;  // gen_packet_strm_from_file already logged
        } else if (t.contains("tone")) {
            const auto& tone = t["tone"];
            float freq = tone.value("freq", 1000.0f);
            float gain = tone.value("gain", 0.3f);
            stream = gen_packet_strm_tone(freq, gain, chann);
        } else {
            std::cerr << "io_sim: track for channel " << chann
                      << " needs either 'path' or 'tone'" << std::endl;
            return -1;
        }
        stems_loop.push_back(std::move(stream));
    }

    // All loops are normalized to the shortest stream so the cursor wraps cleanly.
    size_t min_len = stems_loop.front().size();
    for (const auto& s : stems_loop) min_len = std::min(min_len, s.size());
    if (min_len == 0) {
        std::cerr << "io_sim: a track produced zero packets" << std::endl;
        return -1;
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

        stream_cursor = (stream_cursor + 1) % min_len;

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