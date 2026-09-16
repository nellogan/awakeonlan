#ifndef AWAKEONLAN_HPP
#define AWAKEONLAN_HPP

#include <arpa/inet.h>
#include <unistd.h>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <system_error>
#include <vector>

#include "utils.hpp"

namespace wol {

inline constexpr uint16_t DEFAULT_PORT = 9;

class AWakeOnLAN {
   public:
    explicit AWakeOnLAN(char* ip_addr_arg, char* port_arg, char* mac_arg, char* passwd_arg, bool ipv6_toggle);
    ~AWakeOnLAN();

    AWakeOnLAN(AWakeOnLAN const&) = delete;
    AWakeOnLAN& operator=(AWakeOnLAN const&) = delete;
    AWakeOnLAN(AWakeOnLAN&&) = delete;
    AWakeOnLAN& operator=(AWakeOnLAN&&) = delete;

    void send_magic_packet();

   private:
    void set_ip_addr(char* ip_addr_arg, bool ipv6_toggle_val);

    void config_socket_settings();

    void set_port(char const* port_arg);

    void set_mac(char* mac_arg);

    void set_passwd(char* passwd_arg);

    void create_magic_packet();

    std::string ip_addr_;

    int port_{};

    std::vector<uint8_t> mac_;

    std::vector<uint8_t> passwd_;

    std::vector<uint8_t> magic_packet_;

    int domain_{};

    int socket_fd_{-1};

    sockaddr_in server_address_{};

    sockaddr_in6 server_address_v6_{};
};

}  // namespace wol

#endif  // AWAKEONLAN_HPP