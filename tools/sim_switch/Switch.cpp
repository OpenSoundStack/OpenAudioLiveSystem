#include "Switch.h"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <iostream>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/uio.h>

// Per-conn rx_buf must fit at least one max-size frame. AudioPacket framing on
// the wire is ~360 bytes; budget generously for future EtherTypes too.
constexpr size_t MAX_FRAME_PAYLOAD = 8192;
constexpr size_t RX_READ_CHUNK     = 4096;

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

    if (c.hello_received) {
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
    c.hello_received = true;
    c.rx_buf.erase(c.rx_buf.begin(), c.rx_buf.begin() + sizeof(SimHello));

    uint32_t key = (uint32_t(h.ethertype) << 16) | h.self_uid;
    m_route_table[key] = c.fd;

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

    // Stats
    int idx = (int)etype_index(hdr.ethertype);
    m_stats.frames_in[idx]++;
    m_stats.bytes_in[idx] += hdr.payload_len;
    if (hdr.dest_uid == 0) m_stats.bcast_in[idx]++;

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
    if (hdr.dest_uid == 0) {
        for (auto& target : m_conns) {
            if (target.fd == sender.fd) continue;
            if (!target.hello_received) continue;
            if (target.ethertype != static_cast<EthProtocol>(hdr.ethertype)) continue;
            try_write(target,
                      reinterpret_cast<const uint8_t*>(&hdr), sizeof(hdr),
                      payload, hdr.payload_len);
        }
    } else {
        uint32_t key = (uint32_t(hdr.ethertype) << 16) | hdr.dest_uid;
        auto it = m_route_table.find(key);
        if (it == m_route_table.end()) return;  // unknown UID — drop silently
        if (it->second == sender.fd) return;
        Conn* target = find_conn(it->second);
        if (!target) return;
        try_write(*target,
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

std::vector<Switch::ConnSummary> Switch::conns() const {
    std::vector<ConnSummary> out;
    out.reserve(m_conns.size());
    for (const auto& c : m_conns) {
        out.push_back({c.fd, c.hello_received,
                       static_cast<uint16_t>(c.ethertype),
                       c.self_uid, c.drops});
    }
    return out;
}
