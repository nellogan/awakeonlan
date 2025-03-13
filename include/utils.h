#ifndef UTILS_H
#define UTILS_H

#include <cstdint>
#include <string>
#include <vector>
#include <system_error>

#include <arpa/inet.h>


int DetermineIPVersion(const char* src);

bool ValidateMACAddr(char* mac_addr_str);

void SanitizeMACAddr(char* mac_addr_str);

uint8_t HexDigitToUint8(char str);

void HexStringToBytesVec(std::string& ascii_string, std::vector<uint8_t>& bytes_vec);

#endif