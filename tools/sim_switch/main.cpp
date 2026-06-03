#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "Switch.h"
#include "Tui.h"

namespace {

constexpr const char* DEFAULT_SOCKET_PATH = "/tmp/osst-sim-default.sock";

int g_shutdown_pipe[2] = {-1, -1};
std::atomic<bool> g_shutdown_requested{false};

void on_signal(int) {
    g_shutdown_requested = true;
    if (g_shutdown_pipe[1] >= 0) {
        char b = 'q';
        ssize_t r = ::write(g_shutdown_pipe[1], &b, 1);
        (void)r;
    }
}

void set_nonblock(int fd) {
    int flags = ::fcntl(fd, F_GETFL, 0);
    ::fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

uint64_t now_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

struct Args {
    std::string socket_path = DEFAULT_SOCKET_PATH;
    bool        headless    = false;
};

Args parse_args(int argc, char** argv) {
    Args a;
    for (int i = 1; i < argc; ++i) {
        std::string s = argv[i];
        if (s == "--socket-path" && i + 1 < argc) {
            a.socket_path = argv[++i];
        } else if (s == "--headless") {
            a.headless = true;
        } else if (s == "-h" || s == "--help") {
            std::cout
                << "sim_switch — Unix-socket OAN switch daemon\n"
                << "Usage: sim_switch [--socket-path <path>] [--headless]\n"
                << "  --socket-path  default " << DEFAULT_SOCKET_PATH << "\n"
                << "  --headless     periodic stdout snapshot instead of TUI\n";
            std::exit(0);
        } else {
            std::cerr << "sim_switch: unknown arg '" << s << "'\n";
            std::exit(2);
        }
    }
    return a;
}

int open_listen_socket(const std::string& path) {
    // Unlink stale socket file.
    if (::unlink(path.c_str()) == 0) {
        std::cerr << "sim_switch: unlinked stale socket file " << path << "\n";
    } else if (errno != ENOENT) {
        std::cerr << "sim_switch: warning: could not unlink " << path
                  << ": " << ::strerror(errno) << "\n";
    }

    int fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        std::cerr << "sim_switch: socket() failed: " << ::strerror(errno) << "\n";
        return -1;
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    if (path.size() >= sizeof(addr.sun_path)) {
        std::cerr << "sim_switch: socket path too long\n";
        ::close(fd);
        return -1;
    }
    std::strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);

    if (::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::cerr << "sim_switch: bind() failed: " << ::strerror(errno) << "\n";
        ::close(fd);
        return -1;
    }
    if (::listen(fd, 16) < 0) {
        std::cerr << "sim_switch: listen() failed: " << ::strerror(errno) << "\n";
        ::close(fd);
        ::unlink(path.c_str());
        return -1;
    }

    set_nonblock(fd);
    return fd;
}

} // namespace

int main(int argc, char** argv) {
    Args args = parse_args(argc, argv);

    if (::pipe(g_shutdown_pipe) < 0) {
        std::cerr << "sim_switch: pipe() failed\n";
        return 1;
    }
    set_nonblock(g_shutdown_pipe[0]);
    set_nonblock(g_shutdown_pipe[1]);

    struct sigaction sa{};
    sa.sa_handler = on_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    ::sigaction(SIGINT,  &sa, nullptr);
    ::sigaction(SIGTERM, &sa, nullptr);
    ::signal(SIGPIPE, SIG_IGN);

    int listen_fd = open_listen_socket(args.socket_path);
    if (listen_fd < 0) return 1;

    std::cerr << "sim_switch: listening on " << args.socket_path
              << (args.headless ? " (headless)" : "") << "\n";

    Switch sw;
    Tui    tui(args.socket_path, args.headless);

    std::vector<int> client_fds;

    while (!g_shutdown_requested.load()) {
        std::vector<pollfd> pfds;
        pfds.push_back({listen_fd, POLLIN, 0});
        pfds.push_back({g_shutdown_pipe[0], POLLIN, 0});
        for (int fd : client_fds) {
            pfds.push_back({fd, POLLIN, 0});
        }

        int rc = ::poll(pfds.data(), pfds.size(), 200);
        if (rc < 0) {
            if (errno == EINTR) continue;
            std::cerr << "sim_switch: poll() failed: " << ::strerror(errno) << "\n";
            break;
        }

        uint64_t t = now_ms();
        sw.set_now_ms(t);

        if (rc > 0) {
            // Accept new
            if (pfds[0].revents & POLLIN) {
                while (true) {
                    int cfd = ::accept(listen_fd, nullptr, nullptr);
                    if (cfd < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                        std::cerr << "sim_switch: accept() failed: "
                                  << ::strerror(errno) << "\n";
                        break;
                    }
                    set_nonblock(cfd);
                    sw.on_accept(cfd);
                    client_fds.push_back(cfd);
                }
            }

            // Shutdown pipe
            if (pfds[1].revents & POLLIN) {
                char buf[8];
                ssize_t r = ::read(g_shutdown_pipe[0], buf, sizeof(buf));
                (void)r;
                break;
            }

            // Clients
            std::vector<int> to_remove;
            for (size_t i = 2; i < pfds.size(); ++i) {
                int fd = pfds[i].fd;
                short re = pfds[i].revents;
                if (re & (POLLERR | POLLHUP | POLLNVAL)) {
                    to_remove.push_back(fd);
                    continue;
                }
                if (re & POLLIN) {
                    if (!sw.on_readable(fd)) {
                        to_remove.push_back(fd);
                    }
                }
            }
            for (int fd : to_remove) {
                sw.on_hangup(fd);
                client_fds.erase(std::remove(client_fds.begin(), client_fds.end(), fd),
                                 client_fds.end());
            }
        }

        // Periodic housekeeping: prune stale disco entries, refresh TUI.
        sw.prune_disco(t, 20000);
        tui.refresh(sw, t);
    }

    std::cerr << "\nsim_switch: shutting down\n";
    tui.shutdown();
    for (int fd : client_fds) ::close(fd);
    ::close(listen_fd);
    ::unlink(args.socket_path.c_str());
    ::close(g_shutdown_pipe[0]);
    ::close(g_shutdown_pipe[1]);
    return 0;
}
