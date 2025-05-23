#include "routing_routines.h"

bool control_pipe_create_routing(
    AudioEngine &audio_engine,
    AudioPlumber &plumber,
    AudioRouter &router,
    ControlPipeCreatePacket &pck,
    LowLatHeader &llhdr)
{
    // Check if a pipe is already pending
        if (plumber.pending_elem_count() == 0) {
            // No pipe pending
            plumber.set_pending_client(llhdr.sender_uid);
            plumber.add_elem_to_pending_pipe(pck.packet_data.elem_type, pck.packet_data.stack_position);

        } else if (plumber.get_pending_client() == llhdr.sender_uid) {
            // Client we are working with yay, adding its element to the list
            plumber.add_elem_to_pending_pipe(pck.packet_data.elem_type, pck.packet_data.stack_position);

        } else {
            // A pipe is pending, wrong channel
            // Rejecting pipe creation

            constexpr char err_message[64] = "A pipe is already pending for another client.";

            ControlResponsePacket rej_packet{};
            rej_packet.header.type = PacketType::CONTROL_RESPONSE;
            rej_packet.header.timestamp = pck.header.timestamp;
            rej_packet.packet_data.channel = pck.packet_data.channel;
            rej_packet.packet_data.response = ControlResponseCode::CREATE_ERROR;
            memcpy(rej_packet.packet_data.err_msg, err_message, 64);

            router.send_control_packet(rej_packet, llhdr.sender_uid);

            std::cerr << LOG_PREFIX << "Rejected pipe elem creation. Client mismatch." << std::endl;

            return false;
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
                rej_packet.packet_data.channel = plumber.get_pending_client();
                rej_packet.packet_data.response = CREATE_TYPE_UNK | CREATE_ERROR;
                memcpy(rej_packet.packet_data.err_msg, err_message, 64);

                router.send_control_packet(rej_packet, llhdr.sender_uid);
                std::cerr << LOG_PREFIX << "Failed to instantiate a pipe (creation initiated by control network)" << std::endl;

                return false;
            } else {
                auto allocated_channel = audio_engine.install_pipe(pipeline.value());

                if (allocated_channel.has_value()) {
                    ControlResponsePacket ack_packet{};
                    ack_packet.header.type = PacketType::CONTROL_RESPONSE;
                    ack_packet.header.timestamp = pck.header.timestamp;
                    ack_packet.packet_data.channel = allocated_channel.value();
                    ack_packet.packet_data.response = CREATE_OK;

                    router.send_control_packet(ack_packet, llhdr.sender_uid);

                    std::cout << "Installed new pipeline on channel " << (int)allocated_channel.value() << std::endl;
                    std::cout << "Channel map is " << std::hex << audio_engine.get_channel_usage_map() << std::dec << std::endl;
                } else {
                    std::cerr << "Failed to allocate a channel for the new pipe" << std::endl;

                    constexpr char err_msg[64] = "Failed to allocate a new channel on DSP";

                    ControlResponsePacket rej_packet{};
                    rej_packet.header.type = PacketType::CONTROL_RESPONSE;
                    rej_packet.header.timestamp = pck.header.timestamp;
                    rej_packet.packet_data.channel = 0;
                    rej_packet.packet_data.response = CREATE_ALLOC_FAILED | CREATE_ERROR;
                    memcpy(rej_packet.packet_data.err_msg, err_msg, 64);

                    router.send_control_packet(rej_packet, llhdr.sender_uid);
                }

                return true;
            }
        }

    return false;
}

void reset_dsp_alloc(AudioEngine &engine, AudioRouter& router, LowLatHeader& llhdr) {
    ControlResponsePacket response_packet{};
    response_packet.header.type = PacketType::CONTROL_RESPONSE;
    response_packet.packet_data.channel = 0;

    if (engine.reset_pipes()) {
        std::cout << "Resetted DSP pipe allocation" << std::endl;
        response_packet.packet_data.response = ControlResponseCode::CONTROL_ACK;
    } else {
        const char error_message[64] = "Failed to reset DSP.";
        response_packet.packet_data.response = ControlResponseCode::CONTROL_ERROR;

        memcpy(response_packet.packet_data.err_msg, error_message, 64);
    }

    router.send_control_packet(response_packet, llhdr.sender_uid);
}

