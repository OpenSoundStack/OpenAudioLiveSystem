#ifndef OSST_SIM_PROTO_H
#define OSST_SIM_PROTO_H

#include <cstdint>

// Host-only wire framing between sim_switch (OALS dev tooling) and
// SimTransport (OAN host backend). Length-prefixed binary; host byte order
// (daemon and clients share machine endianness).
//
// NOTE: this is NOT part of the OAN raw-Ethernet wire contract. Embedded
// firmware never builds this; it exists purely so dev hosts (macOS/Linux)
// can run multi-process OALS over AF_UNIX without raw sockets.
//
// v2: SimFrame gained src_uid (switch-populated); SimHello._reserved became
// flags so clients can opt into promiscuous mirroring (used by oaninspect).

constexpr uint32_t SIM_MAGIC   = 0x4F535354;  // 'OSST'
constexpr uint8_t  SIM_VERSION = 2;

// SimHello.flags bits.
constexpr uint16_t SIM_HELLO_PROMISCUOUS = 0x0001;

struct SimHello {
    uint32_t magic;
    uint8_t  version;       // SIM_VERSION; bump on framing change
    uint8_t  _pad;          // reserved, must be 0
    uint16_t ethertype;
    uint16_t self_uid;
    uint16_t flags;         // SIM_HELLO_* bits
} __attribute__((packed));

struct SimFrame {
    uint32_t payload_len;
    uint16_t ethertype;
    uint16_t dest_uid;
    uint16_t src_uid;       // populated by switch from sender's hello
    uint16_t _pad;          // alignment / future flags
} __attribute__((packed));

#endif
