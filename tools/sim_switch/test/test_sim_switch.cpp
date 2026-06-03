#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <cstring>
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

    bool send_hello(uint32_t magic, uint16_t etype, uint16_t uid) {
        SimHello h{magic, SIM_VERSION, 0, etype, uid, 0};
        return ::send(m_fd, &h, sizeof(h), 0) == (ssize_t)sizeof(h);
    }
    bool send_frame(uint16_t etype, uint16_t dest_uid,
                    const std::vector<uint8_t>& payload) {
        SimFrame f{(uint32_t)payload.size(), etype, dest_uid};
        ::send(m_fd, &f, sizeof(f), 0);
        return ::send(m_fd, payload.data(), payload.size(), 0)
               == (ssize_t)payload.size();
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
