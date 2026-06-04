#include "Switch.h"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <iostream>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/uio.h>

#include "common/packet_structs.h"

// Per-conn rx_buf must fit at least one max-size frame. AudioPacket framing on
// the wire is ~360 bytes; budget generously for future EtherTypes too.
constexpr size_t MAX_FRAME_PAYLOAD = 8192;
constexpr size_t RX_READ_CHUNK     = 4096;

// Layout the disco peer puts on the wire (matches DiscoveryPeek): [eth 14]
// [LowLatHeader 6][OANPacket<MappingData>]. self_uid lives inside the
// MappingData payload.
namespace {
constexpr size_t DISCO_LL_PREFIX = 14 + 6;

// Extract self_uid from a disco frame payload if it parses as a MAPPING
// packet; returns 0 otherwise (also for genuinely uid=0 packets, which is
// fine — uid=0 means "unknown" in our adoption flow).
uint16_t parse_mapping_self_uid(const uint8_t* payload, size_t len) {
    if (len < DISCO_LL_PREFIX + sizeof(OANPacket<MappingData>)) return 0;
    OANPacket<MappingData> pck{};
    std::memcpy(&pck, payload + DISCO_LL_PREFIX, sizeof(pck));
    if (pck.header.type != PacketType::MAPPING) return 0;
    return pck.packet_data.self_uid;
}
} // namespace

Switch::EtypeIdx Switch::etype_index(uint16_t e) {
    switch (e) {
        case ETH_PROTO_OANAUDIO:   return EtypeIdx::AUDIO;
        case ETH_PROTO_OANDISCO:   return EtypeIdx::DISCO;
        case ETH_PROTO_OANCONTROL: return EtypeIdx::CONTROL;
        case ETH_PROTO_OANSYNC:    return EtypeIdx::SYNC;
        default:                   return EtypeIdx::OTHER;
    }
}

Switch::Conn* Switch::find_conn(int fd) {
    auto it = m_fd_to_idx.find(fd);
    if (it == m_fd_to_idx.end()) return nullptr;
    return &m_conns[it->second];
}

void Switch::on_accept(int fd) {
    Conn c{};
    c.fd = fd;
    c.rx_buf.reserve(RX_READ_CHUNK);
    m_fd_to_idx[fd] = m_conns.size();
    m_conns.push_back(std::move(c));
}

void Switch::on_hangup(int fd) {
    remove_conn(fd);
}

void Switch::remove_conn(int fd) {
    auto idx_it = m_fd_to_idx.find(fd);
    if (idx_it == m_fd_to_idx.end()) return;
    size_t idx = idx_it->second;
    Conn& c = m_conns[idx];

    if (c.hello_received && !c.promiscuous) {
        uint32_t key = (uint32_t(static_cast<uint16_t>(c.ethertype)) << 16) | c.self_uid;
        auto rt = m_route_table.find(key);
        if (rt != m_route_table.end() && rt->second == fd) {
            m_route_table.erase(rt);
        }
    }

    ::close(fd);

    // Swap-and-pop so we keep O(1) removal. Fix up the swapped conn's index.
    size_t last = m_conns.size() - 1;
    if (idx != last) {
        m_conns[idx] = std::move(m_conns[last]);
        m_fd_to_idx[m_conns[idx].fd] = idx;
    }
    m_conns.pop_back();
    m_fd_to_idx.erase(idx_it);
}

bool Switch::on_readable(int fd) {
    Conn* c = find_conn(fd);
    if (!c) return false;

    uint8_t buf[RX_READ_CHUNK];
    ssize_t n = ::read(fd, buf, sizeof(buf));
    if (n == 0) return false;          // peer closed
    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) return true;
        return false;
    }

    c->rx_buf.insert(c->rx_buf.end(), buf, buf + n);

    // Drain as many complete messages as we have.
    while (true) {
        if (!c->hello_received) {
            int r = consume_hello(*c);
            if (r == 0) break;          // not enough bytes yet
            if (r < 0) return false;    // bad magic → caller closes
            // r > 0 → hello accepted, loop to parse first frame if buffered
        } else {
            int r = consume_frame(*c);
            if (r == 0) break;          // not enough bytes yet
            if (r < 0) return false;    // oversize → caller closes
            // r > 0 → frame delivered, loop to parse the next one if buffered
        }
    }

    return true;
}

int Switch::consume_hello(Conn& c) {
    if (c.rx_buf.size() < sizeof(SimHello)) return 0;

    SimHello h{};
    std::memcpy(&h, c.rx_buf.data(), sizeof(h));

    if (h.magic != SIM_MAGIC) {
        std::cerr << "sim_switch: bad hello magic on fd=" << c.fd
                  << " (got 0x" << std::hex << h.magic << std::dec << "); closing\n";
        return -1;
    }
    if (h.version != SIM_VERSION) {
        std::cerr << "sim_switch: hello version mismatch on fd=" << c.fd
                  << " (got " << (int)h.version << ", expected " << (int)SIM_VERSION
                  << "); closing\n";
        return -1;
    }

    c.ethertype = static_cast<EthProtocol>(h.ethertype);
    c.self_uid  = h.self_uid;
    c.promiscuous = (h.flags & SIM_HELLO_PROMISCUOUS) != 0;
    c.hello_received = true;
    c.rx_buf.erase(c.rx_buf.begin(), c.rx_buf.begin() + sizeof(SimHello));

    // Promiscuous conns are observers, not addressable destinations — keep
    // them out of the route table so they can't intercept unicasts meant
    // for a real peer of the same uid.
    if (!c.promiscuous) {
        uint32_t key = (uint32_t(h.ethertype) << 16) | h.self_uid;
        m_route_table[key] = c.fd;
    }

    return 1;
}

int Switch::consume_frame(Conn& c) {
    if (c.rx_buf.size() < sizeof(SimFrame)) return 0;

    SimFrame hdr{};
    std::memcpy(&hdr, c.rx_buf.data(), sizeof(hdr));

    if (hdr.payload_len > MAX_FRAME_PAYLOAD) {
        std::cerr << "sim_switch: oversize frame from uid=" << c.self_uid
                  << " len=" << hdr.payload_len << "; dropping connection\n";
        return -1;
    }

    if (c.rx_buf.size() < sizeof(SimFrame) + hdr.payload_len) return 0;

    const uint8_t* payload = c.rx_buf.data() + sizeof(SimFrame);

    // Conn-uid adoption: NetworkMapper creates its discovery socket *before*
    // UID autoconfig runs, so the hello on that conn carries uid=0. The
    // committed UID first shows up in the MAPPING packet payload. Adopt it
    // here so per-peer stats, src_uid stamping, and the route table all see
    // the right identity for the rest of the session.
    if (!c.promiscuous && c.self_uid == 0
        && hdr.ethertype == ETH_PROTO_OANDISCO) {
        uint16_t learned = parse_mapping_self_uid(payload, hdr.payload_len);
        if (learned != 0) {
            // Drop the bogus (disco, 0) route only if it still points to us.
            // Another zero-uid conn may have overwritten it at hello time.
            uint32_t old_key = (uint32_t(ETH_PROTO_OANDISCO) << 16) | 0u;
            auto old_it = m_route_table.find(old_key);
            if (old_it != m_route_table.end() && old_it->second == c.fd) {
                m_route_table.erase(old_it);
            }
            c.self_uid = learned;
            uint32_t new_key = (uint32_t(ETH_PROTO_OANDISCO) << 16) | learned;
            m_route_table[new_key] = c.fd;
        }
    }

    // The switch owns src_uid attribution — sender's value is overwritten
    // with the conn's registered uid so observers can trust it.
    hdr.src_uid = c.self_uid;
    // Mirror back into the rx_buf so the bytes fanout writes to peers are
    // the corrected ones, not the sender's zero. fanout reads `hdr` (a
    // local copy) for the header, so updating the buffered copy is purely
    // defensive in case someone later refactors to splat raw bytes.
    std::memcpy(c.rx_buf.data(), &hdr, sizeof(hdr));

    // Stats
    int idx = (int)etype_index(hdr.ethertype);
    m_stats.frames_in[idx]++;
    m_stats.bytes_in[idx] += hdr.payload_len;
    if (hdr.dest_uid == 0) m_stats.bcast_in[idx]++;

    // Per-peer tx attribution. Promiscuous conns can also send (rare, but
    // legal) — they get counted under their hello uid like any peer.
    auto& peer = m_peer_stats[c.self_uid];
    peer.tx[idx]++;
    peer.last_activity_ms = m_now_ms;

    // TUI enrichment (bounded — only disco)
    if (hdr.ethertype == ETH_PROTO_OANDISCO) {
        m_disco.observe(payload, hdr.payload_len, m_now_ms);
    }

    fanout(c, hdr, payload);

    c.rx_buf.erase(c.rx_buf.begin(),
                   c.rx_buf.begin() + sizeof(SimFrame) + hdr.payload_len);
    return 1;
}

void Switch::fanout(const Conn& sender, const SimFrame& hdr, const uint8_t* payload) {
    int idx = (int)etype_index(hdr.ethertype);

    if (hdr.dest_uid == 0) {
        for (auto& target : m_conns) {
            if (target.fd == sender.fd) continue;
            if (!target.hello_received) continue;
            // Promiscuous conns are handled in the mirror pass below so we
            // don't double-deliver to them when their hello ethertype
            // happens to match.
            if (target.promiscuous) continue;
            if (target.ethertype != static_cast<EthProtocol>(hdr.ethertype)) continue;
            try_write(target,
                      reinterpret_cast<const uint8_t*>(&hdr), sizeof(hdr),
                      payload, hdr.payload_len);
            auto& p = m_peer_stats[target.self_uid];
            p.rx[idx]++;
            p.last_activity_ms = m_now_ms;
        }
    } else {
        uint32_t key = (uint32_t(hdr.ethertype) << 16) | hdr.dest_uid;
        auto it = m_route_table.find(key);
        if (it != m_route_table.end() && it->second != sender.fd) {
            Conn* target = find_conn(it->second);
            if (target) {
                try_write(*target,
                          reinterpret_cast<const uint8_t*>(&hdr), sizeof(hdr),
                          payload, hdr.payload_len);
                auto& p = m_peer_stats[target->self_uid];
                p.rx[idx]++;
                p.last_activity_ms = m_now_ms;
            }
        }
        // Fall through to the promiscuous mirror pass even for unknown
        // unicasts so observers see "uid X tried to talk to nobody" traffic.
    }

    // Promiscuous mirror pass — every fanout, regardless of ethertype or
    // dest_uid match. Inspectors don't bump per-peer rx (they're not
    // production endpoints; counting their rx would skew the peers table).
    for (auto& target : m_conns) {
        if (!target.promiscuous) continue;
        if (target.fd == sender.fd) continue;
        if (!target.hello_received) continue;
        try_write(target,
                  reinterpret_cast<const uint8_t*>(&hdr), sizeof(hdr),
                  payload, hdr.payload_len);
    }
}

void Switch::try_write(Conn& target, const uint8_t* hdr_buf, size_t hdr_len,
                       const uint8_t* payload, size_t payload_len) {
    iovec iov[2] = {
        { const_cast<uint8_t*>(hdr_buf), hdr_len },
        { const_cast<uint8_t*>(payload), payload_len }
    };
    msghdr m{};
    m.msg_iov    = iov;
    m.msg_iovlen = 2;

    ssize_t n = ::sendmsg(target.fd, &m, MSG_DONTWAIT
#ifdef __linux__
                          | MSG_NOSIGNAL
#endif
                          );
    if (n < 0) {
        // Any send error counts as a drop. Hangups surface via poll(POLLHUP)
        // separately, so we don't need to remove the conn here.
        target.drops++;
        return;
    }
    if (static_cast<size_t>(n) < hdr_len + payload_len) {
        // Partial write under load: simplest policy is to drop the remainder.
        target.drops++;
    }
}

void Switch::prune_disco(uint64_t now_ms, uint64_t max_age_ms) {
    m_disco.prune(now_ms, max_age_ms);
}

void Switch::prune_peer_stats(uint64_t now_ms, uint64_t max_age_ms) {
    for (auto it = m_peer_stats.begin(); it != m_peer_stats.end(); ) {
        if (it->second.last_activity_ms == 0
            || now_ms - it->second.last_activity_ms > max_age_ms) {
            it = m_peer_stats.erase(it);
        } else {
            ++it;
        }
    }
}

std::vector<Switch::ConnSummary> Switch::conns() const {
    std::vector<ConnSummary> out;
    out.reserve(m_conns.size());
    for (const auto& c : m_conns) {
        out.push_back({c.fd, c.hello_received, c.promiscuous,
                       static_cast<uint16_t>(c.ethertype),
                       c.self_uid, c.drops});
    }
    return out;
}

int Switch::inspector_count() const {
    int n = 0;
    for (const auto& c : m_conns) {
        if (c.hello_received && c.promiscuous) n++;
    }
    return n;
}
