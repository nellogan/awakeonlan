#include <cassert>
#include <cstdio>
#include <cstring>
#include "utils.h"


void DetermineIPVersionTest()
{
    int ip_version = 0;
    char ipv4_addr[] = "127.0.0.1";
    ip_version = DetermineIPVersion(ipv4_addr);
    assert( ip_version == 4 && "Error: DetermineIPVersionTest, DetermineIPVersion ipv4 address parse failed.\n" );

    char ipv6_addr[] = "::01";
    ip_version = DetermineIPVersion(ipv6_addr);
    assert( ip_version == 6 && "Error: DetermineIPVersionTest, DetermineIPVersion ipv6 address parse failed.\n" );

    char fake_addr[] = "192.168:ffff:0001";
    ip_version = DetermineIPVersion(fake_addr);
    assert( ip_version == -1 && "Error: DetermineIPVersionTest, DetermineIPVersion incorrectly accepted an address that is not v4 nor v6.\n" );
}

void ValidateMACAddrTest()
{
    bool valid = false;
    char valid_mac1[] = "c001d00DF00d";
    valid = ValidateMACAddr(valid_mac1);
    assert( valid == true && "Error: ValidateMACAddrTest, ValidateMACAddr failed to parse a valid MAC address.\n" );

    char valid_mac2[] = "AB:4d:8c:bb:12:F4";
    valid = ValidateMACAddr(valid_mac2);
    assert( valid == true && "Error: ValidateMACAddrTest, ValidateMACAddr failed to parse a valid MAC address.\n" );

    char valid_mac3[] = "FF-dD-24-Ee-99-22";
    valid = ValidateMACAddr(valid_mac3);
    assert( valid == true && "Error: ValidateMACAddrTest, ValidateMACAddr failed to parse a valid MAC address.\n" );

    char invalid_mac[] = "ec-22-44-55-66-77-88";
    valid = ValidateMACAddr(invalid_mac);
    assert( valid == false && "Error: ValidateMACAddrTest, ValidateMACAddr failed by accepting an invalid MAC address.\n" );
}

void SanitizeMACAddrTest()
{
    char valid_mac_addr_str1[] = "Ab-22-dd-12-df-88";
    SanitizeMACAddr(valid_mac_addr_str1);
    char check1[] = "Ab22dd12df88";
    assert( (std::strncmp(valid_mac_addr_str1, check1, std::strlen(check1)) == 0) &&
            "Error: SanitizeMACAddrTest, SanitizeMACAddr failed by not properly sanitizing a valid MAC address.\n" );

    char valid_mac_addr_str2[] = "ee:55:31:12:7e:5d";
    SanitizeMACAddr(valid_mac_addr_str2);
    char check2[] = "ee5531127e5d";
    assert( (std::strncmp(valid_mac_addr_str2, check2, std::strlen(check2)) == 0) &&
            "Error: SanitizeMACAddrTest, SanitizeMACAddr failed by not properly sanitizing a valid MAC address.\n" );
}

void HexDigitToUint8Test()
{
    uint8_t result = 0;

    char hex_digit1 = 'c';
    uint8_t check1 = 12;
    result = HexDigitToUint8(hex_digit1);
    assert( result == check1 && "Error: HexDigitToUint8Test, HexDigitToUint8 failed to parse a valid hex digit correctly.\n" );

    char hex_digit2 = 'F';
    uint8_t check2 = 15;
    result = HexDigitToUint8(hex_digit2);
    assert( result == check2 && "Error: HexDigitToUint8Test, HexDigitToUint8 failed to parse a valid hex digit correctly.\n" );

    char hex_digit3 = '4';
    uint8_t check3 = 4;
    result = HexDigitToUint8(hex_digit3);
    assert( result == check3 && "Error: HexDigitToUint8Test, HexDigitToUint8 failed to parse a valid hex digit correctly.\n" );

    char invalid_hex_digit = 'g';
    try
    {
        result = HexDigitToUint8(invalid_hex_digit); // This should throw an exception.
    }
    catch (const std::exception& ex)
    {
        return;
    }
    throw std::invalid_argument("Error: HexDigitToUint8Test, HexDigitToUint8 failed throw an exception for an invalid hex character.\n");
}

void HexStringToBytesVecTest()
{
    std::string hex_string1 = "cafe12";
    std::vector<uint8_t> check_bytes_vec1 = { 0xca, 0xfe, 0x12 };
    std::vector<uint8_t> hex_bytes_vec1;
    HexStringToBytesVec(hex_string1, hex_bytes_vec1);
    assert( hex_bytes_vec1 == check_bytes_vec1 && "Error: HexStringToBytesVecTest, HexStringToBytesVec failed to match check values.\n" );

    std::string hex_string2 = "ff99acd9e2b7";
    std::vector<uint8_t> check_bytes_vec2 = { 0xff, 0x99, 0xac, 0xd9, 0xe2, 0xb7 };
    std::vector<uint8_t> hex_bytes_vec2;
    HexStringToBytesVec(hex_string2, hex_bytes_vec2);
    assert( hex_bytes_vec2 == check_bytes_vec2 && "Error: HexStringToBytesVecTest, HexStringToBytesVec failed to match check values.\n" );
}

int main()
{
    DetermineIPVersionTest();
    ValidateMACAddrTest();
    SanitizeMACAddrTest();
    HexDigitToUint8Test();
    HexStringToBytesVecTest();

    return 0;
}