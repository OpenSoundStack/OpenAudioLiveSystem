#include "DiscoveryPeek.h"

#include <cstring>

// Layout of a disco frame's payload bytes (what the daemon receives in SimFrame
// payload): [ethhdr 14][LowLatHeader 6][OANPacket<MappingData>].
// Skip 20 bytes to land on the OANPacket header.
constexpr size_t LL_HEADERS_SIZE = 14 + 6;

void DiscoveryPeek::observe(const uint8_t* payload, size_t len, uint64_t now_ms) {
    if (len < LL_HEADERS_SIZE + sizeof(OANPacket<MappingData>)) return;

    OANPacket<MappingData> opck{};
    std::memcpy(&opck, payload + LL_HEADERS_SIZE, sizeof(opck));

    if (opck.header.type != PacketType::MAPPING) return;

    auto& d = m_devices[opck.packet_data.self_uid];
    d.uid  = opck.packet_data.self_uid;
    d.mac  = opck.packet_data.self_address;
    d.name = std::string(opck.packet_data.dev_name,
                         strnlen(opck.packet_data.dev_name, 32));
    d.type = opck.packet_data.type;
    d.topo = opck.packet_data.topo;
    d.last_seen_ms = now_ms;
}

void DiscoveryPeek::prune(uint64_t now_ms, uint64_t max_age_ms) {
    for (auto it = m_devices.begin(); it != m_devices.end(); ) {
        if (now_ms - it->second.last_seen_ms > max_age_ms) {
            it = m_devices.erase(it);
        } else {
            ++it;
        }
    }
}
