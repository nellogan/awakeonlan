#include "awakeonlan.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>
#include <system_error>

#include "utils.hpp"

namespace wol {

constexpr char const* DEFAULT_IPV4_BCAST = "255.255.255.255";
constexpr char const* DEFAULT_IPV6_MCAST = "ff02::1";
constexpr int MAX_PORT_NUMBER = 65535;

AWakeOnLAN::AWakeOnLAN(char* ip_addr_arg, char* port_arg, char* mac_arg, char* passwd_arg, bool ipv6_toggle) {
    set_ip_addr(ip_addr_arg, ipv6_toggle);
    set_port(port_arg);
    set_mac(mac_arg);
    config_socket_settings();
    if (passwd_arg != nullptr) {
        set_passwd(passwd_arg);
    }
    create_magic_packet();
}

AWakeOnLAN::~AWakeOnLAN() {
    if (socket_fd_ >= 0) {
        close(socket_fd_);
    }
}

void AWakeOnLAN::set_ip_addr(char* ip_addr_arg, bool ipv6_toggle_val) {
    if (ip_addr_arg == nullptr) {
        ip_addr_ = !ipv6_toggle_val ? DEFAULT_IPV4_BCAST : DEFAULT_IPV6_MCAST;
    } else {
        int const ip_version = determine_ip_version(ip_addr_arg);
        if (ip_version == -1) {
            throw std::invalid_argument("Error: AWakeOnLAN::set_ip_addr, invalid IP Address.");
        }
        ipv6_toggle_val = ip_version != 4;
        ip_addr_ = ip_addr_arg;
    }
    domain_ = ipv6_toggle_val ? AF_INET6 : AF_INET;
}

void AWakeOnLAN::config_socket_settings() {
    socket_fd_ = socket(domain_, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_fd_ < 0) {
        std::error_code const err_code(errno, std::system_category());
        throw std::system_error(err_code, "Error: AWakeOnLAN::config_socket_settings, could not create socket.\n");
    }

    if (domain_ == AF_INET) {
        int broadcast_enable = 1;
        // NOLINTBEGIN(misc-include-cleaner)
        if (setsockopt(socket_fd_, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)) < 0) {
            close(socket_fd_);
            std::error_code const err_code(errno, std::system_category());
            throw std::system_error(err_code,
                                    "Error: AWakeOnLAN::config_socket_settings, could set SO_BROADCAST to socket.\n");
        }
        // NOLINTEND(misc-include-cleaner)
        server_address_.sin_family = static_cast<unsigned short>(domain_);
        server_address_.sin_port = htons(port_);
        server_address_.sin_addr.s_addr = inet_addr(ip_addr_.c_str());
    } else {
        std::memset(&server_address_v6_, 0, sizeof(struct sockaddr_in6));
        server_address_v6_.sin6_family = static_cast<unsigned short>(domain_);
        server_address_v6_.sin6_port = htons(port_);
        inet_pton(domain_, ip_addr_.c_str(), &server_address_v6_.sin6_addr);
    }
}

void AWakeOnLAN::set_port(char const* port_arg) {
    if (port_arg == nullptr) {
        port_ = DEFAULT_PORT;
    } else {
        port_ = static_cast<int>(std::stol(port_arg));
    }

    if (port_ <= 0 || port_ > MAX_PORT_NUMBER) {
        throw std::invalid_argument(
            "Error: AWakeOnLAN::set_port, port_ must be greater than 0 and less than or "
            "equal to 65535.");
    }
}

void AWakeOnLAN::set_mac(char* mac_arg) {
    if (!validate_mac_addr(mac_arg)) {
        throw std::invalid_argument(
            "Error: AWakeOnLAN::set_mac, mac_ Address must be 6 bytes long and formatted such that "
            "1. no spaces between hex bytes, or 2. colons ':' between hex bytes, or "
            "3. hyphens '-' between hex bytes.");
    }
    std::string mac_str(mac_arg);
    if (mac_str.length() >= 3 && (mac_str.at(2) == ':' || mac_str.at(2) == '-')) {
        sanitize_mac_addr(mac_arg);
        mac_str = mac_arg;
    }
    hex_string_to_bytes_vec(mac_str, mac_);
}

void AWakeOnLAN::set_passwd(char* passwd_arg) {
    std::string const passwd_str(passwd_arg);
    passwd_.assign(passwd_str.begin(), passwd_str.end());
}

void AWakeOnLAN::create_magic_packet() {
    magic_packet_.assign(MAC_ADDR_BYTE_SZ, WAKE_PACKET_HEADER_BYTE);
    for (int i = 0; i < WAKE_PACKET_MAC_REPEAT_COUNT; i++) {
        magic_packet_.insert(magic_packet_.end(), mac_.begin(), mac_.end());
    }
    if (!passwd_.empty()) {
        magic_packet_.insert(magic_packet_.end(), passwd_.begin(), passwd_.end());
    }
}

void AWakeOnLAN::send_magic_packet() {
    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    auto const send_err_code =
        (domain_ == AF_INET) ? sendto(socket_fd_, magic_packet_.data(), magic_packet_.size(), 0,
                                      reinterpret_cast<struct sockaddr*>(&server_address_), sizeof(server_address_))
                             : sendto(socket_fd_, magic_packet_.data(), magic_packet_.size(), 0,
                                      reinterpret_cast<struct sockaddr*>(&server_address_v6_), sizeof(sockaddr_in6));
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

    if (send_err_code < 0) {
        std::error_code const err_code(errno, std::system_category());
        throw std::system_error(err_code, "Error: AWakeOnLAN::send_magic_packet, failed to send message.\n");
    }
}

}  // namespace wol
