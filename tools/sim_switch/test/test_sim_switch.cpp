#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <thread>
#include <vector>

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

#include "sim_proto.h"
#include "netutils/transport/SimTransport.h"
#include "netutils/LowLatSocket.h"

using namespace std::chrono_literals;

namespace {

// Returns the daemon binary path from $SIM_SWITCH_BIN, falling back to a
// build-tree-relative default for direct invocation.
std::string daemon_path() {
    const char* p = std::getenv("SIM_SWITCH_BIN");
    if (p && *p) return p;
    return "./sim_switch";
}

std::string make_test_socket_path() {
    return "/tmp/osst-sim-test-" + std::to_string(::getpid()) + "-"
         + std::to_string(::time(nullptr)) + ".sock";
}

// RAII spawn of sim_switch as a child process. Sends SIGTERM on dtor.
// Use `ASSERT_TRUE(d.ready())` in the test body to bail out cleanly on failure.
class DaemonProc {
public:
    explicit DaemonProc(const std::string& socket_path) : m_socket_path(socket_path) {
        m_pid = ::fork();
        if (m_pid < 0) return;
        if (m_pid == 0) {
            std::string bin = daemon_path();
            std::vector<char*> argv = {
                const_cast<char*>(bin.c_str()),
                const_cast<char*>("--headless"),
                const_cast<char*>("--socket-path"),
                const_cast<char*>(socket_path.c_str()),
                nullptr
            };
            ::execv(bin.c_str(), argv.data());
            std::cerr << "execv failed: " << ::strerror(errno) << "\n";
            std::_Exit(127);
        }
        // Parent: wait for daemon to be ready.
        for (int i = 0; i < 200; ++i) {
            int fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
            sockaddr_un addr{};
            addr.sun_family = AF_UNIX;
            std::strncpy(addr.sun_path, socket_path.c_str(), sizeof(addr.sun_path)-1);
            if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0) {
                ::close(fd);
                m_ready = true;
                return;
            }
            ::close(fd);
            std::this_thread::sleep_for(20ms);
        }
    }

    ~DaemonProc() {
        if (m_pid > 0) {
            ::kill(m_pid, SIGTERM);
            int st = 0;
            ::waitpid(m_pid, &st, 0);
        }
        ::unlink(m_socket_path.c_str());
    }

    bool ready() const { return m_ready; }
    pid_t pid() const { return m_pid; }

private:
    pid_t       m_pid{-1};
    std::string m_socket_path;
    bool        m_ready{false};
};

// Raw AF_UNIX client (no SimTransport wrapping) so we can drive corner cases.
class RawClient {
public:
    bool connect(const std::string& path) {
        m_fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
        if (m_fd < 0) return false;
        sockaddr_un a{};
        a.sun_family = AF_UNIX;
        std::strncpy(a.sun_path, path.c_str(), sizeof(a.sun_path)-1);
        return ::connect(m_fd, reinterpret_cast<sockaddr*>(&a), sizeof(a)) == 0;
    }
    ~RawClient() { if (m_fd >= 0) ::close(m_fd); }

    bool send_hello(uint32_t magic, uint16_t etype, uint16_t uid,
                    uint16_t flags = 0) {
        SimHello h{magic, SIM_VERSION, 0, etype, uid, flags};
        return ::send(m_fd, &h, sizeof(h), 0) == (ssize_t)sizeof(h);
    }
    bool send_frame(uint16_t etype, uint16_t dest_uid,
                    const std::vector<uint8_t>& payload) {
        // v2: SimFrame is 12B with src_uid + _pad. Sender writes 0 — switch
        // overwrites src_uid from our hello uid.
        SimFrame f{(uint32_t)payload.size(), etype, dest_uid, 0, 0};
        ::send(m_fd, &f, sizeof(f), 0);
        return ::send(m_fd, payload.data(), payload.size(), 0)
               == (ssize_t)payload.size();
    }
    // For tests that need the SimFrame header back too (e.g. src_uid check).
    struct FrameRead {
        bool ok{false};
        SimFrame hdr{};
        std::vector<uint8_t> body;
    };
    FrameRead read_frame_full(int timeout_ms) {
        FrameRead r;
        pollfd p{m_fd, POLLIN, 0};
        int pr = ::poll(&p, 1, timeout_ms);
        if (pr <= 0) return r;
        size_t got = 0;
        while (got < sizeof(r.hdr)) {
            ssize_t n = ::read(m_fd, reinterpret_cast<uint8_t*>(&r.hdr) + got,
                               sizeof(r.hdr) - got);
            if (n <= 0) return r;
            got += n;
        }
        r.body.resize(r.hdr.payload_len);
        got = 0;
        while (got < r.hdr.payload_len) {
            ssize_t n = ::read(m_fd, r.body.data() + got, r.hdr.payload_len - got);
            if (n <= 0) return r;
            got += n;
        }
        r.ok = true;
        return r;
    }
    // Block up to timeout_ms reading one full frame. Returns payload bytes (or empty on timeout/error).
    std::vector<uint8_t> read_frame(int timeout_ms) {
        pollfd p{m_fd, POLLIN, 0};
        int pr = ::poll(&p, 1, timeout_ms);
        if (pr <= 0) return {};
        SimFrame hdr{};
        size_t got = 0;
        while (got < sizeof(hdr)) {
            ssize_t n = ::read(m_fd, reinterpret_cast<uint8_t*>(&hdr) + got,
                               sizeof(hdr) - got);
            if (n <= 0) return {};
            got += n;
        }
        std::vector<uint8_t> body(hdr.payload_len);
        got = 0;
        while (got < hdr.payload_len) {
            ssize_t n = ::read(m_fd, body.data() + got, hdr.payload_len - got);
            if (n <= 0) return {};
            got += n;
        }
        return body;
    }
    int fd() const { return m_fd; }
    void close_fd() { if (m_fd >= 0) { ::close(m_fd); m_fd = -1; } }

private:
    int m_fd{-1};
};

} // namespace

// 1. Hello round-trip: daemon accepts hellos from two clients.
TEST(SimSwitch, HelloAccepted) {
    auto path = make_test_socket_path();
    DaemonProc d(path);
    ASSERT_TRUE(d.ready());
    RawClient a, b;
    ASSERT_TRUE(a.connect(path));
    ASSERT_TRUE(b.connect(path));
    EXPECT_TRUE(a.send_hello(SIM_MAGIC, ETH_PROTO_OANAUDIO, 1));
    EXPECT_TRUE(b.send_hello(SIM_MAGIC, ETH_PROTO_OANAUDIO, 2));
    // Daemon should not close us; verify by absence of EOF in 100ms.
    pollfd pa{a.fd(), POLLIN, 0};
    EXPECT_EQ(::poll(&pa, 1, 100), 0);
}

// 2. Broadcast on a given EtherType is delivered to all peers except sender.
TEST(SimSwitch, BroadcastFanout) {
    auto path = make_test_socket_path();
    DaemonProc d(path);
    ASSERT_TRUE(d.ready());
    RawClient a, b;
    ASSERT_TRUE(a.connect(path));
    ASSERT_TRUE(b.connect(path));
    a.send_hello(SIM_MAGIC, ETH_PROTO_OANDISCO, 10);
    b.send_hello(SIM_MAGIC, ETH_PROTO_OANDISCO, 11);
    std::this_thread::sleep_for(50ms);  // let daemon register both

    std::vector<uint8_t> payload(100, 0xAB);
    ASSERT_TRUE(a.send_frame(ETH_PROTO_OANDISCO, 0, payload));

    auto got_b = b.read_frame(500);
    ASSERT_EQ(got_b.size(), payload.size());
    EXPECT_EQ(got_b, payload);

    // A should NOT receive its own broadcast.
    auto got_a = a.read_frame(100);
    EXPECT_TRUE(got_a.empty());
}

// 3. Unicast to a known UID is delivered only to that peer.
TEST(SimSwitch, UnicastDelivery) {
    auto path = make_test_socket_path();
    DaemonProc d(path);
    ASSERT_TRUE(d.ready());
    RawClient a, b, c;
    ASSERT_TRUE(a.connect(path));
    ASSERT_TRUE(b.connect(path));
    ASSERT_TRUE(c.connect(path));
    a.send_hello(SIM_MAGIC, ETH_PROTO_OANCONTROL, 42);
    b.send_hello(SIM_MAGIC, ETH_PROTO_OANCONTROL, 51);
    c.send_hello(SIM_MAGIC, ETH_PROTO_OANCONTROL, 99);
    std::this_thread::sleep_for(50ms);

    std::vector<uint8_t> payload{1, 2, 3, 4, 5};
    ASSERT_TRUE(a.send_frame(ETH_PROTO_OANCONTROL, 51, payload));

    auto got_b = b.read_frame(500);
    ASSERT_EQ(got_b.size(), payload.size());
    EXPECT_EQ(got_b, payload);

    auto got_c = c.read_frame(100);
    EXPECT_TRUE(got_c.empty());

    auto got_a = a.read_frame(100);
    EXPECT_TRUE(got_a.empty());
}

// 4. Unicast to an unknown UID is silently dropped.
TEST(SimSwitch, UnknownUnicastDropped) {
    auto path = make_test_socket_path();
    DaemonProc d(path);
    ASSERT_TRUE(d.ready());
    RawClient a, b;
    ASSERT_TRUE(a.connect(path));
    ASSERT_TRUE(b.connect(path));
    a.send_hello(SIM_MAGIC, ETH_PROTO_OANCONTROL, 1);
    b.send_hello(SIM_MAGIC, ETH_PROTO_OANCONTROL, 2);
    std::this_thread::sleep_for(50ms);

    std::vector<uint8_t> payload(10, 0xCC);
    ASSERT_TRUE(a.send_frame(ETH_PROTO_OANCONTROL, 999, payload));

    auto got_b = b.read_frame(100);
    EXPECT_TRUE(got_b.empty());
}

// 5. Hello with bad magic causes the daemon to drop the connection.
TEST(SimSwitch, BadMagicClosesConn) {
    auto path = make_test_socket_path();
    DaemonProc d(path);
    ASSERT_TRUE(d.ready());
    RawClient a;
    ASSERT_TRUE(a.connect(path));
    EXPECT_TRUE(a.send_hello(0xDEADBEEF, ETH_PROTO_OANAUDIO, 1));

    // Daemon should close our socket within ~300ms.
    pollfd p{a.fd(), POLLIN, 0};
    int pr = ::poll(&p, 1, 500);
    ASSERT_GT(pr, 0);
    char buf;
    EXPECT_EQ(::read(a.fd(), &buf, 1), 0);  // EOF from server
}

// 6. Slow client overflows kernel send buffer → daemon increments drops and
//    keeps serving fast clients. We don't have programmatic access to the
//    daemon's stats so the check is liveness: A's writes keep succeeding and
//    a second fast client C still receives traffic.
TEST(SimSwitch, SlowClientDoesNotBlock) {
    auto path = make_test_socket_path();
    DaemonProc d(path);
    ASSERT_TRUE(d.ready());
    RawClient a, b, c;
    ASSERT_TRUE(a.connect(path));
    ASSERT_TRUE(b.connect(path));
    ASSERT_TRUE(c.connect(path));
    a.send_hello(SIM_MAGIC, ETH_PROTO_OANAUDIO, 1);
    b.send_hello(SIM_MAGIC, ETH_PROTO_OANAUDIO, 2);
    c.send_hello(SIM_MAGIC, ETH_PROTO_OANAUDIO, 3);
    std::this_thread::sleep_for(50ms);

    // B never reads. A blasts a large number of broadcast frames.
    std::vector<uint8_t> payload(1024, 0xEE);
    for (int i = 0; i < 200; ++i) {
        ASSERT_TRUE(a.send_frame(ETH_PROTO_OANAUDIO, 0, payload));
    }

    // C should still be able to receive at least some frames — daemon didn't lock up.
    int got = 0;
    for (int i = 0; i < 10; ++i) {
        auto f = c.read_frame(200);
        if (!f.empty()) got++;
    }
    EXPECT_GT(got, 0);
}

// 7. SimTransport (real impl from OAN) interoperates with the daemon.
//    Two SimTransport clients on the same EtherType broadcast → each sees the other.
TEST(SimSwitch, SimTransportInterop) {
    auto path = make_test_socket_path();
    // Daemon's default-socket-path is hardcoded; we must use the matching
    // SimTransport daemon name. Compose: socket_path = /tmp/osst-sim-<name>.sock.
    // Extract <name> from the temp path.
    // Easier: use a fixed name and let DaemonProc translate it.
    std::string daemon_name = "interop-" + std::to_string(::getpid());
    std::string sock = "/tmp/osst-sim-" + daemon_name + ".sock";
    DaemonProc d(sock);
    ASSERT_TRUE(d.ready());

    SimTransport tA(daemon_name);
    SimTransport tB(daemon_name);

    IfaceMeta metaA{}, metaB{};
    ASSERT_TRUE(tA.open("sim:" + daemon_name, ETH_PROTO_OANAUDIO, 7, metaA));
    ASSERT_TRUE(tB.open("sim:" + daemon_name, ETH_PROTO_OANAUDIO, 8, metaB));
    std::this_thread::sleep_for(50ms);

    // tA broadcasts a payload; tB should receive it.
    std::vector<uint8_t> payload(64, 0x77);
    int sent = tA.send(payload.data(), payload.size(), 0);
    EXPECT_EQ(sent, (int)payload.size());

    std::vector<uint8_t> buf(payload.size());
    // Loop briefly for delivery (non-blocking recv may need a couple polls).
    int got = 0;
    for (int i = 0; i < 50; ++i) {
        got = tB.recv(buf.data(), buf.size(), true);
        if (got > 0) break;
        std::this_thread::sleep_for(10ms);
    }
    EXPECT_EQ(got, (int)payload.size());
    EXPECT_EQ(buf, payload);
}

// ------ M5 ----------------------------------------------------------------

// 8. Promiscuous client receives broadcasts on every EtherType regardless of
//    its own hello ethertype.
TEST(SimSwitch, PromiscuousReceivesAllEthertypes) {
    auto path = make_test_socket_path();
    DaemonProc d(path);
    ASSERT_TRUE(d.ready());
    RawClient a, b, c, insp;
    ASSERT_TRUE(a.connect(path));
    ASSERT_TRUE(b.connect(path));
    ASSERT_TRUE(c.connect(path));
    ASSERT_TRUE(insp.connect(path));
    a.send_hello(SIM_MAGIC, ETH_PROTO_OANAUDIO,   100);
    b.send_hello(SIM_MAGIC, ETH_PROTO_OANCONTROL, 42);
    c.send_hello(SIM_MAGIC, ETH_PROTO_OANSYNC,    1);
    insp.send_hello(SIM_MAGIC, 0, 0xFFFE, SIM_HELLO_PROMISCUOUS);
    std::this_thread::sleep_for(50ms);

    a.send_frame(ETH_PROTO_OANAUDIO,   0, std::vector<uint8_t>(50, 0x11));
    b.send_frame(ETH_PROTO_OANCONTROL, 0, std::vector<uint8_t>(50, 0x22));
    c.send_frame(ETH_PROTO_OANSYNC,    0, std::vector<uint8_t>(50, 0x33));

    std::map<uint16_t, int> seen_by_etype;
    for (int i = 0; i < 3; ++i) {
        auto f = insp.read_frame_full(500);
        ASSERT_TRUE(f.ok) << "inspector missed frame " << i;
        seen_by_etype[f.hdr.ethertype]++;
    }
    EXPECT_EQ(seen_by_etype[ETH_PROTO_OANAUDIO],   1);
    EXPECT_EQ(seen_by_etype[ETH_PROTO_OANCONTROL], 1);
    EXPECT_EQ(seen_by_etype[ETH_PROTO_OANSYNC],    1);
}

// 9. Promiscuous client also receives unicasts to other peers' uids.
TEST(SimSwitch, PromiscuousReceivesUnicasts) {
    auto path = make_test_socket_path();
    DaemonProc d(path);
    ASSERT_TRUE(d.ready());
    RawClient a, b, insp;
    ASSERT_TRUE(a.connect(path));
    ASSERT_TRUE(b.connect(path));
    ASSERT_TRUE(insp.connect(path));
    a.send_hello(SIM_MAGIC, ETH_PROTO_OANCONTROL, 100);
    b.send_hello(SIM_MAGIC, ETH_PROTO_OANCONTROL, 200);
    insp.send_hello(SIM_MAGIC, 0, 0xFFFE, SIM_HELLO_PROMISCUOUS);
    std::this_thread::sleep_for(50ms);

    std::vector<uint8_t> payload(64, 0xAB);
    ASSERT_TRUE(a.send_frame(ETH_PROTO_OANCONTROL, 200, payload));

    auto got_b = b.read_frame_full(500);
    ASSERT_TRUE(got_b.ok);
    EXPECT_EQ(got_b.body, payload);
    EXPECT_EQ(got_b.hdr.dest_uid, 200);

    auto got_i = insp.read_frame_full(500);
    ASSERT_TRUE(got_i.ok);
    EXPECT_EQ(got_i.body, payload);
    EXPECT_EQ(got_i.hdr.dest_uid, 200);
    EXPECT_EQ(got_i.hdr.ethertype, ETH_PROTO_OANCONTROL);

    // And to a non-existent dst: only inspector should see it.
    std::vector<uint8_t> orphan(32, 0xEE);
    ASSERT_TRUE(a.send_frame(ETH_PROTO_OANCONTROL, 999, orphan));
    auto got_b2 = b.read_frame_full(100);
    EXPECT_FALSE(got_b2.ok);
    auto got_i2 = insp.read_frame_full(500);
    ASSERT_TRUE(got_i2.ok);
    EXPECT_EQ(got_i2.body, orphan);
    EXPECT_EQ(got_i2.hdr.dest_uid, 999);
}

// 10. Switch overwrites SimFrame::src_uid with the sender's hello uid, even
//     if the sender wrote a different value. Validated via inspector readback.
TEST(SimSwitch, SrcUidPopulatedBySwitch) {
    auto path = make_test_socket_path();
    DaemonProc d(path);
    ASSERT_TRUE(d.ready());
    RawClient a, insp;
    ASSERT_TRUE(a.connect(path));
    ASSERT_TRUE(insp.connect(path));
    a.send_hello(SIM_MAGIC, ETH_PROTO_OANDISCO, 77);
    insp.send_hello(SIM_MAGIC, 0, 0xFFFE, SIM_HELLO_PROMISCUOUS);
    std::this_thread::sleep_for(50ms);

    // Sender writes a lying src_uid by going lower-level than send_frame:
    SimFrame f{16, ETH_PROTO_OANDISCO, 0, /*src_uid LIE=*/0xDEAD, 0};
    ::send(a.fd(), &f, sizeof(f), 0);
    std::vector<uint8_t> body(16, 0x55);
    ::send(a.fd(), body.data(), body.size(), 0);

    auto got = insp.read_frame_full(500);
    ASSERT_TRUE(got.ok);
    EXPECT_EQ(got.hdr.src_uid, 77);  // switch overwrote the lie
    EXPECT_EQ(got.hdr.dest_uid, 0);
    EXPECT_EQ(got.body, body);
}

// 11. Multiple inspectors each see the same frame.
TEST(SimSwitch, MultipleInspectors) {
    auto path = make_test_socket_path();
    DaemonProc d(path);
    ASSERT_TRUE(d.ready());
    RawClient a, i1, i2;
    ASSERT_TRUE(a.connect(path));
    ASSERT_TRUE(i1.connect(path));
    ASSERT_TRUE(i2.connect(path));
    a.send_hello(SIM_MAGIC, ETH_PROTO_OANDISCO, 5);
    i1.send_hello(SIM_MAGIC, 0, 0xFFFE, SIM_HELLO_PROMISCUOUS);
    i2.send_hello(SIM_MAGIC, 0, 0xFFFF, SIM_HELLO_PROMISCUOUS);
    std::this_thread::sleep_for(50ms);

    std::vector<uint8_t> payload(40, 0x42);
    ASSERT_TRUE(a.send_frame(ETH_PROTO_OANDISCO, 0, payload));

    auto g1 = i1.read_frame_full(500);
    auto g2 = i2.read_frame_full(500);
    ASSERT_TRUE(g1.ok);
    ASSERT_TRUE(g2.ok);
    EXPECT_EQ(g1.body, payload);
    EXPECT_EQ(g2.body, payload);
}

// 12. A promiscuous client does NOT receive its own outgoing frames mirrored.
TEST(SimSwitch, PromiscuousNoLoopback) {
    auto path = make_test_socket_path();
    DaemonProc d(path);
    ASSERT_TRUE(d.ready());
    RawClient insp, peer;
    ASSERT_TRUE(insp.connect(path));
    ASSERT_TRUE(peer.connect(path));
    insp.send_hello(SIM_MAGIC, ETH_PROTO_OANDISCO, 0xFFFE, SIM_HELLO_PROMISCUOUS);
    peer.send_hello(SIM_MAGIC, ETH_PROTO_OANDISCO, 50);
    std::this_thread::sleep_for(50ms);

    std::vector<uint8_t> payload(16, 0xCC);
    ASSERT_TRUE(insp.send_frame(ETH_PROTO_OANDISCO, 0, payload));

    // Peer is on disco ethertype → it should receive the broadcast.
    auto got_peer = peer.read_frame_full(500);
    ASSERT_TRUE(got_peer.ok);
    EXPECT_EQ(got_peer.body, payload);

    // Inspector should NOT receive its own broadcast back.
    auto got_self = insp.read_frame_full(200);
    EXPECT_FALSE(got_self.ok);
}

// 13. Promiscuous uid does not steal unicast traffic from a real peer with
//     the same uid. (Edge case: an inspector that happens to register the
//     same uid as a normal peer must not redirect that peer's unicasts.)
TEST(SimSwitch, PromiscuousNotInRouteTable) {
    auto path = make_test_socket_path();
    DaemonProc d(path);
    ASSERT_TRUE(d.ready());
    RawClient a, b, insp;
    ASSERT_TRUE(a.connect(path));
    ASSERT_TRUE(b.connect(path));
    ASSERT_TRUE(insp.connect(path));

    // Inspector registers first, claiming uid=200 with promiscuous flag.
    insp.send_hello(SIM_MAGIC, ETH_PROTO_OANCONTROL, 200, SIM_HELLO_PROMISCUOUS);
    a.send_hello(SIM_MAGIC, ETH_PROTO_OANCONTROL, 100);
    b.send_hello(SIM_MAGIC, ETH_PROTO_OANCONTROL, 200);  // same uid as inspector
    std::this_thread::sleep_for(50ms);

    std::vector<uint8_t> payload(20, 0xAA);
    ASSERT_TRUE(a.send_frame(ETH_PROTO_OANCONTROL, 200, payload));

    // The real peer B must receive the unicast (the inspector must not steal it).
    auto got_b = b.read_frame_full(500);
    ASSERT_TRUE(got_b.ok);
    EXPECT_EQ(got_b.body, payload);
    EXPECT_EQ(got_b.hdr.dest_uid, 200);

    // Inspector also receives it via the mirror pass.
    auto got_i = insp.read_frame_full(500);
    ASSERT_TRUE(got_i.ok);
    EXPECT_EQ(got_i.body, payload);
}

// 14. Wire version mismatch is rejected: v1 hello (older SIM_VERSION value)
//     causes the daemon to close.
TEST(SimSwitch, V1HelloRejected) {
    auto path = make_test_socket_path();
    DaemonProc d(path);
    ASSERT_TRUE(d.ready());
    RawClient a;
    ASSERT_TRUE(a.connect(path));

    // Construct a v1 hello (8 bytes: magic+v1+pad+etype+uid+reserved).
    // Sending the v2 hello with version=1 reaches the same parse path.
    SimHello h{SIM_MAGIC, /*version=*/1, 0, ETH_PROTO_OANAUDIO, 1, 0};
    ASSERT_EQ(::send(a.fd(), &h, sizeof(h), 0), (ssize_t)sizeof(h));

    pollfd p{a.fd(), POLLIN, 0};
    int pr = ::poll(&p, 1, 500);
    ASSERT_GT(pr, 0);
    char buf;
    EXPECT_EQ(::read(a.fd(), &buf, 1), 0);  // EOF from server
}

// 16. Disco conn UID adoption. NetworkMapper creates its discovery socket
// before UID autoconfig runs, so the hello on that conn carries uid=0. Once
// the autoconfigured UID first appears in a MAPPING packet, the switch must
// adopt it as the conn's identity — otherwise unicast disco to that peer
// goes nowhere and the TUI shows every engine aggregated as uid=0.
#include "common/packet_structs.h"
TEST(SimSwitch, DiscoConnAdoptsUidFromMappingPayload) {
    auto path = make_test_socket_path();
    DaemonProc d(path);
    ASSERT_TRUE(d.ready());
    RawClient a, b, insp;
    ASSERT_TRUE(a.connect(path));
    ASSERT_TRUE(b.connect(path));
    ASSERT_TRUE(insp.connect(path));

    // A hellos with uid=0 (the bootstrap-conn case). B is a normal peer.
    a.send_hello(SIM_MAGIC, ETH_PROTO_OANDISCO, 0);
    b.send_hello(SIM_MAGIC, ETH_PROTO_OANDISCO, 99);
    insp.send_hello(SIM_MAGIC, 0, 0xFFFE, SIM_HELLO_PROMISCUOUS);
    std::this_thread::sleep_for(50ms);

    // A broadcasts a MAPPING packet announcing committed_uid=42. Match the
    // wire layout DiscoveryPeek parses: [eth 14][LowLatHeader 6][OANPacket].
    constexpr uint16_t COMMITTED_UID = 42;
    constexpr size_t PREFIX = 14 + 6;
    std::vector<uint8_t> payload(PREFIX + sizeof(OANPacket<MappingData>), 0);
    OANPacket<MappingData> pck{};
    pck.header.type = PacketType::MAPPING;
    pck.packet_data.self_uid = COMMITTED_UID;
    std::memcpy(payload.data() + PREFIX, &pck, sizeof(pck));
    ASSERT_TRUE(a.send_frame(ETH_PROTO_OANDISCO, /*dest=*/0, payload));

    // Inspector confirms the broadcast went out with src_uid=42, proving
    // the conn's identity flipped from 0 to 42 in the switch's bookkeeping.
    auto got = insp.read_frame_full(500);
    ASSERT_TRUE(got.ok);
    EXPECT_EQ(got.hdr.src_uid, COMMITTED_UID);

    // B unicasts disco to dest=42. This only routes if the switch added a
    // (disco, 42) → A entry to the route table — the second half of adoption.
    std::vector<uint8_t> hello_a(8, 0xAB);
    ASSERT_TRUE(b.send_frame(ETH_PROTO_OANDISCO, COMMITTED_UID, hello_a));
    auto from_b = a.read_frame_full(500);
    ASSERT_TRUE(from_b.ok);
    EXPECT_EQ(from_b.hdr.src_uid, 99);
    EXPECT_EQ(from_b.hdr.dest_uid, COMMITTED_UID);
    EXPECT_EQ(from_b.body, hello_a);
}

// ------ Filter parser unit tests ------------------------------------------
// These exercise the oaninspect filter expression parser as a pure unit,
// no daemon/process involved.

#include "../../oaninspect/Filter.h"

TEST(InspectFilter, EmptyExprMatchesAll) {
    Filter f; std::string err;
    EXPECT_TRUE(f.parse("", err));
    EXPECT_TRUE(f.empty());
    EXPECT_TRUE(f.match(0, 1, 2));
    EXPECT_TRUE(f.match(4, 0, 0));
}

TEST(InspectFilter, EthertypeMaskAndOr) {
    Filter f; std::string err;
    ASSERT_TRUE(f.parse("ethertype=disco,control", err));
    EXPECT_FALSE(f.match(0, 1, 2));  // audio rejected
    EXPECT_TRUE(f.match(1, 1, 2));   // disco
    EXPECT_TRUE(f.match(2, 1, 2));   // control
    EXPECT_FALSE(f.match(3, 1, 2));  // sync rejected
}

TEST(InspectFilter, PeerMatchesEitherSide) {
    Filter f; std::string err;
    ASSERT_TRUE(f.parse("peer=42", err));
    EXPECT_TRUE(f.match(0, 42, 99));
    EXPECT_TRUE(f.match(0, 99, 42));
    EXPECT_FALSE(f.match(0, 99, 100));
}

TEST(InspectFilter, MixedConditions) {
    Filter f; std::string err;
    ASSERT_TRUE(f.parse("ethertype=audio,src=51", err));
    EXPECT_TRUE(f.match(0, 51, 100));   // audio + src=51
    EXPECT_FALSE(f.match(0, 52, 100));  // audio but wrong src
    EXPECT_FALSE(f.match(2, 51, 100));  // src=51 but wrong ethertype
}

TEST(InspectFilter, BadKeyReportsError) {
    Filter f; std::string err;
    EXPECT_FALSE(f.parse("nope=1", err));
    EXPECT_NE(err.find("unknown filter key"), std::string::npos);
}

TEST(InspectFilter, BadEthertypeReportsError) {
    Filter f; std::string err;
    EXPECT_FALSE(f.parse("ethertype=quack", err));
    EXPECT_NE(err.find("unknown ethertype"), std::string::npos);
}

// ------ EWMA unit test ----------------------------------------------------
#include "../Tui.h"

TEST(Ewma, SmoothsAlternatingInput) {
    Ewma e;
    // Push alternating 1/0 with default α=0.15. After ~20 samples the
    // smoothed value should be near 0.5 ± small.
    for (int i = 0; i < 200; ++i) {
        e.update(i % 2 ? 1.0 : 0.0);
    }
    EXPECT_GT(e.smoothed, 0.35);
    EXPECT_LT(e.smoothed, 0.65);
}

TEST(Ewma, SeedsFromFirstSample) {
    Ewma e;
    e.update(42.0);
    EXPECT_DOUBLE_EQ(e.smoothed, 42.0);
}

TEST(Ewma, ConvergesToConstant) {
    Ewma e;
    for (int i = 0; i < 100; ++i) e.update(7.0);
    EXPECT_NEAR(e.smoothed, 7.0, 0.01);
}
