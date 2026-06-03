#ifndef OSST_SIM_SWITCH_DISCOVERY_PEEK_H
#define OSST_SIM_SWITCH_DISCOVERY_PEEK_H

#include <cstdint>
#include <cstddef>
#include <string>
#include <unordered_map>

#include "common/packet_structs.h"

struct DeviceEntry {
    uint16_t     uid{0};
    uint64_t     mac{0};
    std::string  name;
    DeviceType   type{};
    NodeTopology topo{};
    uint64_t     last_seen_ms{0};
};

class DiscoveryPeek {
public:
    void observe(const uint8_t* payload, size_t len, uint64_t now_ms);
    void prune(uint64_t now_ms, uint64_t max_age_ms);

    const std::unordered_map<uint16_t, DeviceEntry>& devices() const { return m_devices; }

private:
    std::unordered_map<uint16_t, DeviceEntry> m_devices;
};

#endif
