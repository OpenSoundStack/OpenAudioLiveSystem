#include <iostream>

#include "AudioEngine.h"
#include "log.h"
#include "NetMan.h"
#include "piping/AudioPlumber.h"

#include "OpenAudioNetwork/common/base_pipes/AudioInPipe.h"
#include "OpenAudioNetwork/common/base_pipes/LevelMeasurePipe.h"
#include "piping/filtering/FiltHPFPipe.h"
#include "piping/filtering/FiltLPFPipe.h"

#include "OpenAudioNetwork/common/AudioRouter.h"

void register_pipes(AudioPlumber* plumber) {
    plumber->register_pipe_element("audioin", []() {
        return std::make_shared<AudioInPipe>();
    });

    plumber->register_pipe_element("dbmeas", []() {
        return std::make_shared<LevelMeasurePipe>();
    });

    plumber->register_pipe_element("hpf1", []() {
        return std::make_shared<FiltHPFPipe>();
    });

    plumber->register_pipe_element("lpf1", [](){
        return std::make_shared<FiltLPFPipe>();
    });
}

int main() {
    AudioPlumber plumber{};
    AudioEngine audio_engine{};
    NetMan nman{&plumber};

    AudioRouter router{100};

    if (!nman.init_netman()) {
        std::cerr << LOG_PREFIX << "Failed to initialize network manager." << std::endl;
    }

    register_pipes(&plumber);

    if (!router.init_router("virbr0", nman.get_net_mapper())) {
        std::cerr << LOG_PREFIX << "Failed to initialize audio router." << std::endl;
        exit(-2);
    }

    router.set_routing_callback([&audio_engine](AudioPacket& pck, LowLatHeader& llhdr) {
        audio_engine.feed_pipe(pck);
    });

    router.set_pipe_create_callback([&audio_engine, &plumber, &router](ControlPipeCreatePacket& pck, LowLatHeader& llhdr) {
        std::cout << "Pipe to create " << (int)pck.packet_data.seq << "/" << (int)pck.packet_data.seq_max
                  << " @ channel " << (int)pck.packet_data.channel << " of type " << pck.packet_data.elem_type << std::endl;

        // Check if a pipe is already pending
        if (plumber.pending_elem_count() == 0) {
            // No pipe pending
            plumber.set_pending_channel(pck.packet_data.channel);
            plumber.add_elem_to_pending_pipe(pck.packet_data.elem_type, pck.packet_data.stack_position);

        } else if (plumber.get_pending_channel() == pck.packet_data.channel) {
            // Channel we are working on yay, adding its element to the list
            plumber.add_elem_to_pending_pipe(pck.packet_data.elem_type, pck.packet_data.stack_position);

        } else {
            // A pipe is pending, wrong channel
            // Rejecting pipe creation

            constexpr char err_message[64] = "A pipe is already pending for another channel.";

            ControlResponsePacket rej_packet{};
            rej_packet.header.type = PacketType::CONTROL_RESPONSE;
            rej_packet.header.timestamp = pck.header.timestamp;
            rej_packet.packet_data.channel = pck.packet_data.channel;
            rej_packet.packet_data.response = ControlResponseCode::CREATE_ERROR;
            memcpy(rej_packet.packet_data.err_msg, err_message, 64);

            router.send_control_packet_response(rej_packet, llhdr.sender_uid);

            std::cerr << LOG_PREFIX << "Rejected pipe elem creation. Channel mismatch." << std::endl;

            return;
        }

        // If we are here, we are sure we have added a new element
        // Checking if we have everything
        if (plumber.pending_elem_count() == pck.packet_data.seq_max) {
            auto pipeline = plumber.construct_pending_pipe();

            if (!pipeline.has_value()) {
                constexpr char err_message[64] = "Failed to instantiate pipe. Type error.";

                ControlResponsePacket rej_packet{};
                rej_packet.header.type = PacketType::CONTROL_RESPONSE;
                rej_packet.header.timestamp = pck.header.timestamp;
                rej_packet.packet_data.channel = plumber.get_pending_channel();
                rej_packet.packet_data.response = ControlResponseCode::CREATE_TYPE_UNK | ControlResponseCode::CREATE_ERROR;
                memcpy(rej_packet.packet_data.err_msg, err_message, 64);

                router.send_control_packet_response(rej_packet, llhdr.sender_uid);
                std::cerr << LOG_PREFIX << "Failed to instantiate a pipe (creation initiated by control network)" << std::endl;
            } else {
                audio_engine.install_pipe(plumber.get_pending_channel(), pipeline.value());

                ControlResponsePacket ack_packet{};
                ack_packet.header.type = PacketType::CONTROL_RESPONSE;
                ack_packet.header.timestamp = pck.header.timestamp;
                ack_packet.packet_data.channel = plumber.get_pending_channel();
                ack_packet.packet_data.response = ControlResponseCode::CREATE_OK;

                router.send_control_packet_response(ack_packet, llhdr.sender_uid);

                std::cout << "Installed new pipeline on channel " << plumber.get_pending_channel() << std::endl;
            }
        }
    });

    if (audio_engine.init_engine() != INIT_OK) {
        std::cerr << LOG_PREFIX << " Failed to initialize audio engine..." << std::endl;
        return -1;
    }

    std::cout << "Initialized Audio Engine." << std::endl;
    std::cout << AUDIO_ENGINE_MAX_PIPES << " pipes available." << std::endl;

    while (true) {
        router.poll_audio_data();
        router.poll_control_packets();
        nman.update_netman();
    }

    return 0;
}