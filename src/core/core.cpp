#include "core.hpp"

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

namespace core {
    std::mutex              MySocket::mtx;
    std::condition_variable MySocket::cv;
    bool                    MySocket::run = false;

    MySocket::MySocket(int family, sa_family_t type, in_port_t port, in_addr_t address) {
        if (type != SOCK_DGRAM) {
            LOGGER(ERR_NOT_UDP);
        }

        sfd = socket(family, type, PROTOCOL);
        if (sfd == -1) {
            LOGGER(ERR_SOCKET);
        }

        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family      = DOMAIN;
        addr.sin_port        = port;
        addr.sin_addr.s_addr = address;
    }

    void MySocket::listener() {
        if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
            LOGGER(ERR_BIND);
        }

        {
            std::lock_guard<std::mutex> lock(mtx);
            run = true;
        }
        cv.notify_one();

        sockaddr  sender_addr;
        socklen_t sender_addr_len = sizeof(sender_addr);

        while (true) {
            char buf[32];
            if (recvfrom(sfd, buf, 32, 0, (struct sockaddr *)&sender_addr, &sender_addr_len) ==
                -1) {
                LOGGER(ERR_READING);
            }

            long t2 = std::chrono::steady_clock::now().time_since_epoch().count();
            std::memcpy(buf, &t2, sizeof(t2));

            long t3 = std::chrono::steady_clock::now().time_since_epoch().count();
            std::memcpy(buf + 8, &t3, sizeof(t3));

            if (sendto(sfd, buf, 32, 0, (struct sockaddr *)&sender_addr, sizeof(sender_addr)) ==
                -1) {
                LOGGER(ERR_WRITING);
            }
        }
    }

    void MySocket::sender() {
        while (true) {
            std::string t1 =
                std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
            std::printf("T1: %s\n", t1.data());

            if (sendto(sfd, t1.data(), t1.size(), 0, (struct sockaddr *)&addr, sizeof(addr)) ==
                -1) {
                LOGGER(ERR_WRITING);
            }

            char buf[32];
            if (read(sfd, buf, 32) == -1) {
                LOGGER(ERR_READING);
            }

            long t4 = std::chrono::steady_clock::now().time_since_epoch().count();
            long t2, t3;

            std::memcpy(&t2, buf, sizeof(t2));
            std::memcpy(&t3, buf + 8, sizeof(t3));

            std::printf("T2: %ld\nT3: %ld\n", t2, t3);
            std::printf("T4: %ld\n", t4);

            long maxi = std::max(t2 - std::stol(t1), t4 - t3);
            long mini = std::min(t2 - std::stol(t1), t4 - t3);

            std::printf("RTT: %ld\n", (t4 - std::stol(t1)));
            std::printf("Width: %ld\n", (maxi - mini));
            std::printf("MAX: %ld\n", maxi);
            std::printf("MIN: %ld\n\n", mini);

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    MySocket::~MySocket() { close(sfd); }
} // namespace core
