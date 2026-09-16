#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstdint>
#include <string>
#include <vector>

namespace wol {

inline constexpr size_t MAC_ADDR_BYTE_SZ = 6;
inline constexpr int MAC_ADDR_DIGIT_COUNT = 12;
inline constexpr int WAKE_PACKET_MAC_REPEAT_COUNT = 16;
inline constexpr uint8_t WAKE_PACKET_HEADER_BYTE = 255;

inline constexpr int IP_VERSION_4 = 4;
inline constexpr int IP_VERSION_6 = 6;

int determine_ip_version(char const* src);

bool validate_mac_addr(char const* mac_addr_str);

void sanitize_mac_addr(char* mac_addr_str);

uint8_t hex_digit_to_uint8(char str);

void hex_string_to_bytes_vec(std::string const& ascii_string, std::vector<uint8_t>& bytes_vec);

}  // namespace wol

#endif  // UTILS_HPP