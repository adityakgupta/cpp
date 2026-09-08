#include "utils.hpp"

#include <arpa/inet.h>
#include <cstdint>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/socket.h>

namespace utils {
    namespace {
        // generic handler for user args
        void handler(std::string s = "Invalid arguments") { throw std::invalid_argument(s); }

        in_port_t parse_port(std::string s) {
            int n = std::stoi(s);

            if (n < 1 || n > 65535) {
                handler("Invalid port");
            }

            return htons((uint16_t)n);
        }

        in_addr_t parse_addr(std::string s) {
            in_addr_t addr;

            if (inet_pton(AF_INET, s.c_str(), &addr) < 1) {
                handler("Invalid address");
            }

            return addr;
        }
    } // namespace

    ArgParser::ArgParser(int length, char *args[]) {
        for (int i = 1; i < length; i++) {
            std::string_view arg = args[i];

            if (arg == "--listen") {
                if ((i + 1) >= length) {
                    handler("listen can't have more than 1 args");
                }

                cfg_.listen_port = parse_port(args[++i]);
            } else if (arg == "--send") {
                if ((i + 2) >= length) {
                    handler("send can't have more than 2 args");
                }

                cfg_.send_to = {parse_addr(args[++i]), parse_port(args[++i])};
            } else {
                handler();
            }
        }

        if (cfg_.listen_port.has_value() && cfg_.send_to.has_value()) {
            cfg_.status = BOTH;
        } else if (cfg_.listen_port.has_value()) {
            cfg_.status = LISTENER;
        } else if (cfg_.send_to.has_value()) {
            cfg_.status = SENDER;
        } else {
            handler("no configs");
        }
    }

    const Config &ArgParser::config() const & noexcept { return cfg_; }
} // namespace utils
