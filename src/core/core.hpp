#pragma once

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

#define ERR_NOT_UDP -1
#define ERR_SOCKET  -2
#define ERR_BIND    -3
#define ERR_READING -4
#define ERR_WRITING -5

// exit-strategy only for system-level irrecoverable errors
#define LOGGER(macro)                                                                              \
    do {                                                                                           \
        fprintf(stderr, "%s\t%s\t%s:%d\n", strerror(errno), #macro, __FILE__, __LINE__);           \
        exit(macro);                                                                               \
    } while (0)

#define DOMAIN         AF_INET
#define SOCKTYPE       SOCK_DGRAM
#define PROTOCOL       0
#define LISTEN_BACKLOG 50

namespace core {
    class MySocket {
        private:
            int                sfd;
            struct sockaddr_in addr;

        public:
            static std::mutex              mtx;
            static std::condition_variable cv;
            static bool                    run;
            MySocket(int, sa_family_t, in_port_t, in_addr_t);
            void listener();
            void sender();
            ~MySocket();
    };
} // namespace core
