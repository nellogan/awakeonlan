#include "awakeonlan.hpp"

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <array>
#include <cassert>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <exception>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>

#include "utils.hpp"

namespace {

constexpr size_t RECV_BUFFER_SIZE = 1024;

std::vector<uint8_t> expected_magic_packet(char const* mac_arg, char const* passwd_arg) {
    if (mac_arg == nullptr) {
        return {};
    }
    std::string mac_str(mac_arg);

    if (mac_str.size() >= 3 && (mac_str[2] == ':' || mac_str[2] == '-')) {
        wol::sanitize_mac_addr(mac_str.data());
        mac_str.resize(wol::MAC_ADDR_DIGIT_COUNT);
    }

    std::vector<uint8_t> mac;
    wol::hex_string_to_bytes_vec(mac_str, mac);

    std::string const passwd_str = (passwd_arg != nullptr) ? std::string(passwd_arg) : std::string();

    std::vector<uint8_t> packet;
    packet.reserve(wol::MAC_ADDR_BYTE_SZ + (wol::WAKE_PACKET_MAC_REPEAT_COUNT * mac.size()) + passwd_str.length());

    packet.assign(wol::MAC_ADDR_BYTE_SZ, wol::WAKE_PACKET_HEADER_BYTE);

    for (int i = 0; i < wol::WAKE_PACKET_MAC_REPEAT_COUNT; ++i) {
        packet.insert(packet.end(), mac.begin(), mac.end());
    }

    if (!passwd_str.empty()) {
        packet.insert(packet.end(), passwd_str.begin(), passwd_str.end());
    }

    return packet;
}

int create_receiver_socket(char const* ip_addr, int port) {
    int const domain = wol::determine_ip_version(ip_addr) == wol::IP_VERSION_4 ? AF_INET : AF_INET6;

    int const socket_fd = socket(domain, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_fd < 0) {
        std::error_code const err_code(errno, std::system_category());
        throw std::system_error(err_code, "create_receiver_socket: socket failed");
    }

    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    if (domain == AF_INET) {
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_port = htons(static_cast<uint16_t>(port));
        address.sin_addr.s_addr = INADDR_ANY;

        if (bind(socket_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
            close(socket_fd);
            std::error_code const err_code(errno, std::system_category());
            throw std::system_error(err_code, "create_receiver_socket: bind failed");
        }
    } else {
        sockaddr_in6 address{};
        address.sin6_family = AF_INET6;
        address.sin6_port = htons(static_cast<uint16_t>(port));
        address.sin6_addr = in6addr_any;

        if (bind(socket_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
            close(socket_fd);
            std::error_code const err_code(errno, std::system_category());
            throw std::system_error(err_code, "create_receiver_socket: bind failed");
        }
    }
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

    return socket_fd;
}

void verify_packet(int socket_fd, std::vector<uint8_t> const& expected_packet) {
    std::array<uint8_t, RECV_BUFFER_SIZE> buffer{};

    ssize_t const bytes_received = recvfrom(socket_fd, buffer.data(), buffer.size(), 0, nullptr, nullptr);

    if (bytes_received < 0) {
        close(socket_fd);
        std::error_code const err_code(errno, std::system_category());
        throw std::system_error(err_code, "verify_packet: recvfrom failed");
    }

    close(socket_fd);

    assert(static_cast<size_t>(bytes_received) == expected_packet.size());
    assert(std::memcmp(buffer.data(), expected_packet.data(), expected_packet.size()) == 0);
}

void run_system_test(char* ip_addr_arg, char* port_arg, char* mac_arg, char* passwd_arg, bool ipv6_toggle_arg) {
    int const port = port_arg == nullptr ? wol::DEFAULT_PORT : std::stoi(port_arg);

    int const receiver_fd = create_receiver_socket(ip_addr_arg, port);
    std::vector<uint8_t> const expected = expected_magic_packet(mac_arg, passwd_arg);

    wol::AWakeOnLAN awakeonlan(ip_addr_arg, port_arg, mac_arg, passwd_arg, ipv6_toggle_arg);

    awakeonlan.send_magic_packet();

    verify_packet(receiver_fd, expected);
}

// void system_test_1() {
//     constexpr size_t ipv4_short_addr_buffer_size = 10;
//     constexpr size_t formatted_mac_buffer_size = 18;
//
//     std::array<char, ipv4_short_addr_buffer_size> ip_addr_arg{"127.0.0.1"};
//     char* port_arg = nullptr;
//     std::array<char, formatted_mac_buffer_size> mac_arg{"AB:22:CD:FF:33:77"};
//     char* passwd_arg = nullptr;
//     bool const ipv6_toggle_arg = false;
//
//     run_system_test(
//         ip_addr_arg.data(),
//         port_arg,
//         mac_arg.data(),
//         passwd_arg,
//         ipv6_toggle_arg);
// }

void system_test_2() {
    constexpr size_t ipv4_addr_buffer_size = 16;
    constexpr size_t port_string_buffer_size = 6;
    constexpr size_t formatted_mac_buffer_size = 18;
    constexpr size_t password_buffer_size = 7;

    std::array<char, ipv4_addr_buffer_size> ip_addr_arg{"127.254.255.255"};
    std::array<char, port_string_buffer_size> port_arg{"62000"};
    std::array<char, formatted_mac_buffer_size> mac_arg{"CC-d2-3D-22-99-Aa"};
    std::array<char, password_buffer_size> passwd_arg{"passwd"};

    run_system_test(ip_addr_arg.data(), port_arg.data(), mac_arg.data(), passwd_arg.data(), false);
}

void system_test_3() {
    constexpr size_t ipv6_loopback_buffer_size = 4;
    constexpr size_t port_string_buffer_size = 6;
    constexpr size_t raw_mac_buffer_size = 13;
    constexpr size_t password_buffer_size = 7;

    std::array<char, ipv6_loopback_buffer_size> ip_addr_arg{"::1"};
    std::array<char, port_string_buffer_size> port_arg{"63000"};
    std::array<char, raw_mac_buffer_size> mac_arg{"abcdEf123456"};
    std::array<char, password_buffer_size> passwd_arg{"passwd"};

    run_system_test(ip_addr_arg.data(), port_arg.data(), mac_arg.data(), passwd_arg.data(), false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Use the commented out functions below for troubleshooting on a live network.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// constexpr char TARGET_MACHINE_IP[] = "XXX.XXX.XXX.XXX";
// std::array<char, 18> target_mac_addr{"E0-D5-5E-6B-84-3B"};
//
// void ipv4_test() {
//     constexpr size_t ipv4_addr_buffer_size = 16;
//     constexpr size_t port_string_buffer_size = 2;
//
//     std::array<char, ipv4_addr_buffer_size> ip_addr_arg{TARGET_MACHINE_IP};
//     std::array<char, port_string_buffer_size> port_arg{"9"};
//     char* mac_arg = target_mac_addr.data();
//     char* passwd_arg = nullptr;
//     bool const ipv6_toggle_arg = false;
//
//     wol::AWakeOnLAN awakeonlan(
//         ip_addr_arg.data(),
//         port_arg.data(),
//         mac_arg,
//         passwd_arg,
//         ipv6_toggle_arg);
//
//     awakeonlan.send_magic_packet();
// }
//
// void ipv4_broadcast_test() {
//     char* ip_addr_arg = nullptr;
//     char* port_arg = nullptr;
//     char* mac_arg = target_mac_addr.data();
//     char* passwd_arg = nullptr;
//     bool const ipv6_toggle_arg = false;
//
//     wol::AWakeOnLAN awakeonlan(
//         ip_addr_arg,
//         port_arg,
//         mac_arg,
//         passwd_arg,
//         ipv6_toggle_arg);
//
//     awakeonlan.send_magic_packet();
// }
//
// void ipv6_multicast_test() {
//     constexpr size_t ipv6_mcast_buffer_size = 8;
//     constexpr size_t port_string_buffer_size = 6;
//
//     std::array<char, ipv6_mcast_buffer_size> ip_addr_arg{"ff02::1"};
//     std::array<char, port_string_buffer_size> port_arg{"60000"};
//     char* mac_arg = target_mac_addr.data();
//     char* passwd_arg = nullptr;
//     bool const ipv6_toggle_arg = false;
//
//     wol::AWakeOnLAN awakeonlan(
//         ip_addr_arg.data(),
//         port_arg.data(),
//         mac_arg,
//         passwd_arg,
//         ipv6_toggle_arg);
//
//     awakeonlan.send_magic_packet();
// }

}  // namespace

int main() {
    try {
        // NOTE: system_test_1 uses DEFAULT_PORT(9) which is a privileged port and requires
        // root or superuser permissions.
        // system_test_1();

        system_test_2();

        // NOTE: system_test_3 uses the IPv6 loopback address "::1"; therefore, the system
        // running this test requires IPv6 support.
        system_test_3();

        // ipv4_test();
        // ipv4_broadcast_test();
        // ipv6_multicast_test();
    } catch (std::exception const& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }

    return 0;
}