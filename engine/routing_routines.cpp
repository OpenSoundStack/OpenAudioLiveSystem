#include "routing_routines.h"

void control_pipe_create_routing(
    AudioEngine &audio_engine,
    AudioPlumber &plumber,
    AudioRouter &router,
    ControlPipeCreatePacket &pck,
    LowLatHeader &llhdr)
{
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

            router.send_control_packet(rej_packet, llhdr.sender_uid);

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

                router.send_control_packet(rej_packet, llhdr.sender_uid);
                std::cerr << LOG_PREFIX << "Failed to instantiate a pipe (creation initiated by control network)" << std::endl;
            } else {
                audio_engine.install_pipe(plumber.get_pending_channel(), pipeline.value());

                ControlResponsePacket ack_packet{};
                ack_packet.header.type = PacketType::CONTROL_RESPONSE;
                ack_packet.header.timestamp = pck.header.timestamp;
                ack_packet.packet_data.channel = plumber.get_pending_channel();
                ack_packet.packet_data.response = ControlResponseCode::CREATE_OK;

                router.send_control_packet(ack_packet, llhdr.sender_uid);

                std::cout << "Installed new pipeline on channel " << plumber.get_pending_channel() << std::endl;
            }
        }
}
