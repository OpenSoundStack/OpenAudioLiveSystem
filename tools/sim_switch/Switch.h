#ifndef OSST_SIM_SWITCH_SWITCH_H
#define OSST_SIM_SWITCH_SWITCH_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include <unordered_map>

#include "netutils/LowLatSocket.h"   // EthProtocol enum
#include "sim_proto.h"
#include "DiscoveryPeek.h"

class Switch {
public:
    enum class EtypeIdx : int {
        AUDIO = 0,
        DISCO = 1,
        CONTROL = 2,
        SYNC = 3,
        OTHER = 4,
        COUNT = 5
    };

    struct Stats {
        uint64_t frames_in[(int)EtypeIdx::COUNT]{};
        uint64_t bytes_in[(int)EtypeIdx::COUNT]{};
        uint64_t bcast_in[(int)EtypeIdx::COUNT]{};
    };

    struct ConnSummary {
        int      fd;
        bool     hello_received;
        uint16_t ethertype;
        uint16_t self_uid;
        uint64_t drops;
    };

    void on_accept(int fd);
    void on_hangup(int fd);
    // Returns false if the conn should be closed (bad hello, framing error).
    bool on_readable(int fd);

    const Stats& stats() const { return m_stats; }
    const DiscoveryPeek& disco() const { return m_disco; }
    std::vector<ConnSummary> conns() const;

    void set_now_ms(uint64_t now_ms) { m_now_ms = now_ms; }
    void prune_disco(uint64_t now_ms, uint64_t max_age_ms);

    static EtypeIdx etype_index(uint16_t e);

private:
    struct Conn {
        int                  fd{-1};
        bool                 hello_received{false};
        bool                 must_close{false};
        EthProtocol          ethertype{};
        uint16_t             self_uid{0};
        std::vector<uint8_t> rx_buf;
        uint64_t             drops{0};
    };

    Conn* find_conn(int fd);
    void  remove_conn(int fd);
    // Returns: 1 = accepted, 0 = need more bytes, -1 = bad → close conn.
    int   consume_hello(Conn& c);
    // Returns: 1 = delivered, 0 = need more bytes, -1 = bad → close conn.
    int   consume_frame(Conn& c);
    void  fanout(const Conn& sender, const SimFrame& hdr, const uint8_t* payload);
    void  try_write(Conn& target, const uint8_t* hdr_buf, size_t hdr_len,
                    const uint8_t* payload, size_t payload_len);

    std::vector<Conn> m_conns;
    // fd → index into m_conns. Kept in sync with m_conns to give O(1)
    // lookup on the hot path (every readable event and unicast fanout).
    std::unordered_map<int, size_t>   m_fd_to_idx;
    // key = (ethertype << 16) | dest_uid → conn fd
    std::unordered_map<uint32_t, int> m_route_table;

    DiscoveryPeek m_disco;
    Stats         m_stats;
    uint64_t      m_now_ms{0};
};

#endif
