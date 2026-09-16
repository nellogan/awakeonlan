#include "utils.hpp"

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

namespace {

void determine_ip_version_test() {
    int ip_version = 0;
    std::string const ipv4_addr{"127.0.0.1"};
    ip_version = wol::determine_ip_version(ipv4_addr.c_str());
    assert(ip_version == wol::IP_VERSION_4 &&
           "Error: determine_ip_versionTest, determine_ip_version ipv4 address parse failed.\n");

    std::string const ipv6_addr{"::01"};
    ip_version = wol::determine_ip_version(ipv6_addr.c_str());
    assert(ip_version == wol::IP_VERSION_6 &&
           "Error: determine_ip_versionTest, determine_ip_version ipv6 address parse failed.\n");

    std::string const fake_addr{"192.168:ffff:0001"};
    ip_version = wol::determine_ip_version(fake_addr.c_str());
    assert(ip_version == -1 &&
           "Error: determine_ip_versionTest, determine_ip_version incorrectly accepted an address that is not v4 nor "
           "v6.\n");
}

void validate_mac_addr_test() {
    bool valid = false;
    std::string const valid_mac1{"c001d00DF00d"};
    valid = wol::validate_mac_addr(valid_mac1.data());
    assert(valid == true && "Error: validate_mac_addrTest, validate_mac_addr failed to parse a valid MAC address.\n");

    std::string const valid_mac2{"AB:4d:8c:bb:12:F4"};
    valid = wol::validate_mac_addr(valid_mac2.data());
    assert(valid == true && "Error: validate_mac_addrTest, validate_mac_addr failed to parse a valid MAC address.\n");

    std::string const valid_mac3{"FF-dD-24-Ee-99-22"};
    valid = wol::validate_mac_addr(valid_mac3.data());
    assert(valid == true && "Error: validate_mac_addrTest, validate_mac_addr failed to parse a valid MAC address.\n");

    std::string const invalid_mac{"ec-22-44-55-66-77-88"};
    valid = wol::validate_mac_addr(invalid_mac.data());
    assert(valid == false &&
           "Error: validate_mac_addrTest, validate_mac_addr failed by accepting an invalid MAC address.\n");
}

void sanitize_mac_addr_test() {
    std::string valid_mac_addr_str1{"Ab-22-dd-12-df-88"};
    wol::sanitize_mac_addr(valid_mac_addr_str1.data());
    valid_mac_addr_str1.resize(wol::MAC_ADDR_DIGIT_COUNT);
    std::string const check1{"Ab22dd12df88"};
    assert((valid_mac_addr_str1 == check1) &&
           "Error: sanitize_mac_addrTest, sanitize_mac_addr failed by not properly sanitizing a valid MAC address.\n");

    std::string valid_mac_addr_str2{"ee:55:31:12:7e:5d"};
    wol::sanitize_mac_addr(valid_mac_addr_str2.data());
    valid_mac_addr_str2.resize(wol::MAC_ADDR_DIGIT_COUNT);
    std::string const check2{"ee5531127e5d"};
    assert((valid_mac_addr_str2 == check2) &&
           "Error: sanitize_mac_addrTest, sanitize_mac_addr failed by not properly sanitizing a valid MAC address.\n");
}

void hex_digit_to_uint8_test() {
    uint8_t result = 0;

    char const hex_digit1 = 'c';
    uint8_t const check1 = 12;
    result = wol::hex_digit_to_uint8(hex_digit1);
    assert(result == check1 &&
           "Error: hex_digit_to_uint8Test, hex_digit_to_uint8 failed to parse a valid hex digit correctly.\n");

    char const hex_digit2 = 'F';
    uint8_t const check2 = 15;
    result = wol::hex_digit_to_uint8(hex_digit2);
    assert(result == check2 &&
           "Error: hex_digit_to_uint8Test, hex_digit_to_uint8 failed to parse a valid hex digit correctly.\n");

    char const hex_digit3 = '4';
    uint8_t const check3 = 4;
    result = wol::hex_digit_to_uint8(hex_digit3);
    assert(result == check3 &&
           "Error: hex_digit_to_uint8Test, hex_digit_to_uint8 failed to parse a valid hex digit correctly.\n");

    char const invalid_hex_digit = 'g';
    result = wol::hex_digit_to_uint8(invalid_hex_digit);
    assert(result == 0 &&
           "Error: hex_digit_to_uint8Test, hex_digit_to_uint8 failed to return the fallback value for an invalid hex "
           "character.\n");
}

void hex_string_to_bytes_vec_test() {
    std::string const hex_string1{"cafe12"};
    std::vector<uint8_t> const check_bytes_vec1 = {0xca, 0xfe, 0x12};
    std::vector<uint8_t> hex_bytes_vec1;
    wol::hex_string_to_bytes_vec(hex_string1, hex_bytes_vec1);
    assert(hex_bytes_vec1 == check_bytes_vec1 &&
           "Error: hex_string_to_bytes_vecTest, hex_string_to_bytes_vec failed to match check values.\n");

    std::string const hex_string2{"ff99acd9e2b7"};
    std::vector<uint8_t> const check_bytes_vec2 = {0xff, 0x99, 0xac, 0xd9, 0xe2, 0xb7};
    std::vector<uint8_t> hex_bytes_vec2;
    wol::hex_string_to_bytes_vec(hex_string2, hex_bytes_vec2);
    assert(hex_bytes_vec2 == check_bytes_vec2 &&
           "Error: hex_string_to_bytes_vecTest, hex_string_to_bytes_vec failed to match check values.\n");
}

}  // namespace

int main() {
    determine_ip_version_test();
    validate_mac_addr_test();
    sanitize_mac_addr_test();
    hex_digit_to_uint8_test();
    hex_string_to_bytes_vec_test();

    return 0;
}