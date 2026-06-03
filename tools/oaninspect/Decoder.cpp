#include "Decoder.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <sstream>
#include <string>

#include "common/packet_structs.h"
#include "netutils/LowLatSocket.h"

namespace {

// Same layout dance as sim_switch's DiscoveryPeek: the SimFrame payload
// starts at the ethhdr the engine emits, then the LowLatHeader, then
// the OANPacket.
constexpr size_t LL_HEADERS_SIZE = 14 + 6;  // ethhdr + LowLatHeader

// ANSI colour helpers — guarded by DecodeOpts::color.
struct Color {
    const char* on;
    const char* off;
};
constexpr Color C_AUDIO   = {"\033[36m", "\033[0m"};  // cyan
constexpr Color C_DISCO   = {"\033[32m", "\033[0m"};  // green
constexpr Color C_CONTROL = {"\033[33m", "\033[0m"};  // yellow
constexpr Color C_SYNC    = {"\033[35m", "\033[0m"};  // magenta
constexpr Color C_UNKNOWN = {"\033[31m", "\033[0m"};  // red

Color color_for_etype(uint8_t idx) {
    switch (idx) {
        case 0: return C_AUDIO;
        case 1: return C_DISCO;
        case 2: return C_CONTROL;
        case 3: return C_SYNC;
        default: return C_UNKNOWN;
    }
}

const char* device_type_name(DeviceType t) {
    switch (t) {
        case DeviceType::CONTROL_SURFACE:    return "CONTROL_SURFACE";
        case DeviceType::MONITORING:         return "MONITORING";
        case DeviceType::AUDIO_IO_INTERFACE: return "AUDIO_IO";
        case DeviceType::AUDIO_DSP:          return "AUDIO_DSP";
    }
    return "?";
}

const char* control_query_name(ControlQueryType q) {
    switch (q) {
        case ControlQueryType::PHY_OUT_MAP:      return "PHY_OUT_MAP";
        case ControlQueryType::PIPES_MAP:        return "PIPES_MAP";
        case ControlQueryType::PIPE_ALLOC_RESET: return "PIPE_ALLOC_RESET";
    }
    return "?";
}

const char* packet_type_name(PacketType t) {
    switch (t) {
        case PacketType::MAPPING:          return "MAPPING";
        case PacketType::CONTROL:          return "CONTROL";
        case PacketType::CONTROL_CREATE:   return "CTRL_CREATE";
        case PacketType::CONTROL_RESPONSE: return "CTRL_RESPONSE";
        case PacketType::CONTROL_QUERY:    return "CTRL_QUERY";
        case PacketType::AUDIO:            return "AUDIO";
        case PacketType::CLOCK_SYNC:       return "CLOCK_SYNC";
    }
    return "?";
}

std::string format_timestamp(uint64_t now_ms) {
    char buf[16];
    time_t sec = now_ms / 1000;
    int ms = static_cast<int>(now_ms % 1000);
    struct tm tm_local;
    localtime_r(&sec, &tm_local);
    char hh[16];
    std::strftime(hh, sizeof(hh), "%H:%M:%S", &tm_local);
    std::snprintf(buf, sizeof(buf), "%s.%03d", hh, ms);
    return buf;
}

std::string hex_dump(const uint8_t* p, size_t n, size_t max_bytes) {
    std::ostringstream o;
    size_t lim = n < max_bytes ? n : max_bytes;
    o << "  [";
    for (size_t i = 0; i < lim; ++i) {
        if (i) o << ' ';
        char b[4];
        std::snprintf(b, sizeof(b), "%02x", p[i]);
        o << b;
    }
    if (n > lim) o << " ...(+" << (n - lim) << "B)";
    o << "]";
    return o.str();
}

std::string mac_to_string(uint64_t mac_u64) {
    uint8_t b[8];
    std::memcpy(b, &mac_u64, 8);
    char buf[24];
    std::snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x",
                  b[0], b[1], b[2], b[3], b[4], b[5]);
    return buf;
}

// Pretty-print the OAN payload AFTER the ethhdr+LowLatHeader (20 bytes).
// Returns the body text; the caller wraps with the standard envelope.

std::string decode_audio(const uint8_t* oan, size_t oan_len) {
    if (oan_len < sizeof(AudioPacket)) {
        std::ostringstream o;
        o << "(truncated audio: " << oan_len << "B)";
        return o.str();
    }
    AudioPacket p{};
    std::memcpy(&p, oan, sizeof(p));

    float mn = 1e9f, mx = -1e9f, sumsq = 0.0f;
    for (int i = 0; i < AUDIO_DATA_SAMPLES_PER_PACKETS; ++i) {
        float s = p.packet_data.samples[i];
        if (s < mn) mn = s;
        if (s > mx) mx = s;
        sumsq += s * s;
    }
    float rms = std::sqrt(sumsq / AUDIO_DATA_SAMPLES_PER_PACKETS);

    std::ostringstream o;
    o << "ch=" << std::dec << (int)p.packet_data.channel
      << "  ts=" << p.header.timestamp
      << "  " << AUDIO_DATA_SAMPLES_PER_PACKETS << "smp"
      << "  min=" << mn << "  max=" << mx << "  rms=" << rms;
    return o.str();
}

std::string decode_disco(const uint8_t* oan, size_t oan_len) {
    if (oan_len < sizeof(MappingPacket)) {
        std::ostringstream o;
        o << "(truncated disco: " << oan_len << "B)";
        return o.str();
    }
    MappingPacket p{};
    std::memcpy(&p, oan, sizeof(p));

    if (p.header.type != PacketType::MAPPING) {
        std::ostringstream o;
        o << "type=" << packet_type_name(p.header.type) << " (not MAPPING)";
        return o.str();
    }

    std::string name(p.packet_data.dev_name,
                     strnlen(p.packet_data.dev_name, 32));
    std::ostringstream o;
    o << "uid=" << p.packet_data.self_uid
      << "  mac=" << mac_to_string(p.packet_data.self_address)
      << "  name=\"" << name << "\""
      << "  type=" << device_type_name(p.packet_data.type)
      << "  in=" << (int)p.packet_data.topo.phy_in_count
      << "  out=" << (int)p.packet_data.topo.phy_out_count
      << "  pipes=" << (int)p.packet_data.topo.pipes_count;
    return o.str();
}

std::string decode_control(const uint8_t* oan, size_t oan_len) {
    // Control comes in multiple flavours — switch on the common header's type.
    if (oan_len < sizeof(CommonHeader)) {
        std::ostringstream o;
        o << "(truncated control: " << oan_len << "B)";
        return o.str();
    }
    CommonHeader hdr{};
    std::memcpy(&hdr, oan, sizeof(hdr));

    std::ostringstream o;
    o << "type=" << packet_type_name(hdr.type);

    switch (hdr.type) {
        case PacketType::CONTROL: {
            if (oan_len < sizeof(ControlPacket)) { o << "  (truncated)"; break; }
            ControlPacket p{};
            std::memcpy(&p, oan, sizeof(p));
            o << "  ch=" << (int)p.packet_data.channel
              << "  elem=" << (int)p.packet_data.elem_index
              << "  ctrl=" << p.packet_data.control_id;
            break;
        }
        case PacketType::CONTROL_CREATE: {
            if (oan_len < sizeof(ControlPipeCreatePacket)) { o << "  (truncated)"; break; }
            ControlPipeCreatePacket p{};
            std::memcpy(&p, oan, sizeof(p));
            std::string elem(p.packet_data.elem_type,
                             strnlen(p.packet_data.elem_type, 32));
            o << "  ch=" << (int)p.packet_data.channel
              << "  pid=" << p.packet_data.pid
              << "  seq=" << (int)p.packet_data.seq << "/" << (int)p.packet_data.seq_max
              << "  elem=\"" << elem << "\"";
            break;
        }
        case PacketType::CONTROL_RESPONSE: {
            if (oan_len < sizeof(ControlResponsePacket)) { o << "  (truncated)"; break; }
            ControlResponsePacket p{};
            std::memcpy(&p, oan, sizeof(p));
            o << "  ch=" << (int)p.packet_data.channel
              << "  pid=" << p.packet_data.pid
              << "  code=" << (int)p.packet_data.response;
            break;
        }
        case PacketType::CONTROL_QUERY: {
            if (oan_len < sizeof(ControlQueryPacket)) { o << "  (truncated)"; break; }
            ControlQueryPacket p{};
            std::memcpy(&p, oan, sizeof(p));
            o << "  qtype=" << control_query_name(p.packet_data.qtype)
              << "  flags=0x" << std::hex << p.packet_data.flags << std::dec;
            break;
        }
        default:
            o << "  (unexpected on control ethertype)";
            break;
    }
    return o.str();
}

std::string decode_sync(const uint8_t* oan, size_t oan_len) {
    if (oan_len < sizeof(ClockSyncPacket)) {
        std::ostringstream o;
        o << "(truncated sync: " << oan_len << "B)";
        return o.str();
    }
    ClockSyncPacket p{};
    std::memcpy(&p, oan, sizeof(p));
    std::ostringstream o;
    o << "ts=" << p.header.timestamp
      << "  state=" << (int)p.packet_data.packet_state;
    return o.str();
}

} // namespace

uint8_t etype_index_of(uint16_t e) {
    switch (e) {
        case ETH_PROTO_OANAUDIO:   return 0;
        case ETH_PROTO_OANDISCO:   return 1;
        case ETH_PROTO_OANCONTROL: return 2;
        case ETH_PROTO_OANSYNC:    return 3;
        default:                   return 4;
    }
}

std::string decode_frame_line(uint16_t ethertype,
                              uint16_t src_uid,
                              uint16_t dst_uid,
                              const uint8_t* payload,
                              size_t payload_len,
                              uint64_t now_ms,
                              const DecodeOpts& opts) {
    uint8_t idx = etype_index_of(ethertype);
    Color c = color_for_etype(idx);

    std::ostringstream o;
    if (opts.color) o << c.on;

    o << format_timestamp(now_ms);

    const char* label = "??     ";
    switch (idx) {
        case 0: label = "audio  "; break;
        case 1: label = "disco  "; break;
        case 2: label = "control"; break;
        case 3: label = "sync   "; break;
        default: {
            // Unknown ethertype — render hex code in label slot.
            char buf[12];
            std::snprintf(buf, sizeof(buf), "0x%04x ", ethertype);
            label = buf;
            break;
        }
    }
    o << "  " << label;

    char dst_buf[16];
    std::snprintf(dst_buf, sizeof(dst_buf), "%5u", dst_uid);
    char src_buf[16];
    std::snprintf(src_buf, sizeof(src_buf), "%5u", src_uid);
    o << "  src=" << src_buf << "  dst=" << dst_buf;

    // Body — depends on ethertype. Skip ethhdr + LowLatHeader on payload.
    const uint8_t* oan = nullptr;
    size_t         oan_len = 0;
    if (payload_len >= LL_HEADERS_SIZE) {
        oan = payload + LL_HEADERS_SIZE;
        oan_len = payload_len - LL_HEADERS_SIZE;
    }

    o << "  ";
    if (!oan) {
        o << "(short payload " << payload_len << "B)";
    } else {
        switch (idx) {
            case 0: o << decode_audio(oan, oan_len); break;
            case 1: o << decode_disco(oan, oan_len); break;
            case 2: o << decode_control(oan, oan_len); break;
            case 3: o << decode_sync(oan, oan_len); break;
            default: o << "payload=" << payload_len << "B"; break;
        }
    }

    if (opts.hex && payload) {
        o << hex_dump(payload, payload_len, 48);
    }

    if (opts.color) o << c.off;
    return o.str();
}
