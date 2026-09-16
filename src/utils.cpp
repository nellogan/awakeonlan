#include "utils.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <array>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace wol {

constexpr int MAC_ADDR_SEPARATOR_COUNT = 5;
constexpr uint8_t HEX_VAL_OFFSET = 10;

int determine_ip_version(char const* src) {
    std::array<char, INET6_ADDRSTRLEN> buf{};
    if (inet_pton(AF_INET, src, buf.data()) != 0) {
        return IP_VERSION_4;
    }
    if (inet_pton(AF_INET6, src, buf.data()) != 0) {
        return IP_VERSION_6;
    }
    return -1;
}

bool validate_mac_addr(char const* mac_addr_str) {
    if (mac_addr_str == nullptr) {
        return false;
    }
    std::string const mac_str(mac_addr_str);
    int digit_count = 0;
    int separator_count = 0;
    for (char const character : mac_str) {
        if (isxdigit(static_cast<unsigned char>(character)) != 0) {
            digit_count++;
        } else if (character == ':' || character == '-') {
            if (digit_count == 0 || ((digit_count / 2) - 1) != separator_count) {
                break;
            }
            separator_count++;
        } else {
            separator_count--;
        }
    }
    return (digit_count == MAC_ADDR_DIGIT_COUNT &&
            ((separator_count == MAC_ADDR_SEPARATOR_COUNT) || (separator_count == 0)));
}

void sanitize_mac_addr(char* mac_addr_str) {
    if (mac_addr_str == nullptr) {
        return;
    }
    std::string const mac_str(mac_addr_str);
    std::string cleaned;
    cleaned.reserve(MAC_ADDR_DIGIT_COUNT);
    for (char const character : mac_str) {
        if (character != ':' && character != '-') {
            cleaned.push_back(character);
        }
    }
    std::memcpy(mac_addr_str, cleaned.c_str(), cleaned.size() + 1);
}

uint8_t hex_digit_to_uint8(char str) {
    if (str >= '0' && str <= '9') {
        return static_cast<uint8_t>(str - '0');
    }
    if (str >= 'A' && str <= 'F') {
        return static_cast<uint8_t>(str - 'A' + HEX_VAL_OFFSET);
    }
    if (str >= 'a' && str <= 'f') {
        return static_cast<uint8_t>(str - 'a' + HEX_VAL_OFFSET);
    }
    return 0;  // Since input is pre-validated by validate_mac_addr, this is never hit under normal execution
}

void hex_string_to_bytes_vec(std::string const& ascii_string, std::vector<uint8_t>& bytes_vec) {
    bytes_vec.clear();
    bytes_vec.reserve(ascii_string.length() / 2);

    for (size_t i = 0; i < ascii_string.length(); i += 2) {
        uint8_t const high = hex_digit_to_uint8(ascii_string[i]);
        uint8_t const low = hex_digit_to_uint8(ascii_string[i + 1]);
        bytes_vec.push_back(static_cast<uint8_t>((high << 4) | low));
    }
}

}  // namespace wol
