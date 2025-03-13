#include "utils.h"


int DetermineIPVersion(const char *src)
{
    char buf[INET6_ADDRSTRLEN];
    if (inet_pton(AF_INET, src, buf))
    {
        return 4;
    }
    else if (inet_pton(AF_INET6, src, buf))
    {
        return 6;
    }
    return -1;
}

bool ValidateMACAddr(char* mac_addr_str)
{
    int i = 0;
    int s = 0;
    // Make sure mac_addr_str is null terminated.
    while (*mac_addr_str)
    {
        if (isxdigit(*mac_addr_str))
        {
            i++;
        }
        else if (*mac_addr_str == ':' || *mac_addr_str == '-')
        {
            if (i == 0 || (i / 2 - 1) != s)
            {
                break;
            }
            s++;
        }
        else
        {
            s--;
        }
        ++mac_addr_str;
    }
    return ( i == 12 && ((s == 5) || (s == 0)) );
}

// Use after ValidateMACAddr if (*mac_addr_str+2 == ':' ||  *mac_addr_str+2 == '-').
void SanitizeMACAddr(char* mac_addr_str)
{
    mac_addr_str += 2;
    char* fast_ptr = mac_addr_str+1;
    for (int j=0; j<5; j++)
    {
        *mac_addr_str++ = *fast_ptr++;
        *mac_addr_str++ = *fast_ptr++;
        fast_ptr++;
    }
    *mac_addr_str = '\0';
}

uint8_t HexDigitToUint8(char str)
{
    if (str <= '9' && str >= '0')
    {
        return str - '0';
    }
    if (str <= 'F' && str >= 'A')
    {
        return str - 'A' + 10;
    }
    if ( str <= 'f' && str >= 'a')
    {
        return str - 'a' + 10;
    }
    throw std::invalid_argument("HexDigitToUint8: Invalid hex character\n");
}

void HexStringToBytesVec(std::string& ascii_string, std::vector<uint8_t>& bytes_vec)
{
    int num_bytes = static_cast<int>(ascii_string.length());
    for (int i = 0; i < num_bytes; i += 2)
    {
        uint8_t upper = HexDigitToUint8(ascii_string.at(i));
        uint8_t lower = HexDigitToUint8(ascii_string.at(i+1));
        uint8_t byte = (upper<<4) | lower;
        bytes_vec.push_back(byte);
    }
}
