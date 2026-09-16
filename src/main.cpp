#include <argp.h>

#include <array>

#include "awakeonlan.hpp"

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
char const* argp_program_version = "awakeonlan 0.0.1";
char const* argp_program_bug_address = "...";
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

constexpr char const* DOC =
    "awakeonlan -- sends a WakeOnLAN packet to a MAC address on current Local Area Network or to an IPv4/IPv6 address "
    "over the internet (UDP packet). By default, awakeonlan will broadcast the packet (255.255.255.255, port 9) if IP "
    "address and port is not set. If the ipv6_mcast_toggle switch (-6) is activated with no IP address provided, the "
    "IPv6 "
    "multicast address ff02::1 will be used. Normally a remote host on a LAN will only listen to broadcast/multicast "
    "packets "
    "when in a sleep or unpowered state; Therefore, specify an IP address when waking over the internet (may need to "
    "configure port forwarding on remote hosts network). Supports sending an optional SecureOn password. Supported MAC "
    "address "
    "formats: XX:XX:XX:XX:XX:XX, XX-XX-XX-XX-XX-XX, XXXXXXXXXXXX (where XX is a valid hexadecimal value).";

constexpr char const* ARGS_DOC = "MAC_ADDRESS";

constexpr std::array<argp_option, 5> OPTIONS{{
    {"ip_addr", 'i', "IP_ADDR", 0,
     "IP Address of machine of where the WakeOnLAN packet will be sent. "
     "If unspecified, IP_ADDR=255.255.255.255",
     0},
    {"port", 'p', "PORT", 0, "Port of machine of where the WakeOnLAN packet will be sent", 0},
    {"passwd", 'x', "PASSWD", 0, "SecureOn password, usually six hexadecimal bytes (12 ascii characters)", 0},
    {"ipv6_mcast_toggle", '6', nullptr, 0,
     "If IP_ADDR is omitted, sets IP Address to the IPv6 multicast address "
     "(ff02::1). This option is ignored if IP_ADDR is set.",
     0},
    {nullptr, 0, nullptr, 0, nullptr, 0},
}};

struct Arguments {
    char* mac_arg{nullptr};
    char* ip_addr{nullptr};
    char* port{nullptr};
    char* passwd{nullptr};
    bool ipv6_bcast_toggle{false};
};

namespace {

int parse_opt(int key, char* arg, struct argp_state* state) {
    auto* arguments = static_cast<Arguments*>(state->input);

    switch (key) {
        case 'i':
            arguments->ip_addr = arg;
            break;
        case 'p':
            arguments->port = arg;
            break;
        case 'x':
            arguments->passwd = arg;
            break;
        case '6':
            arguments->ipv6_bcast_toggle = true;
            break;
        case ARGP_KEY_ARG:
            if (state->arg_num >= 1) {
                argp_usage(state);  // NOLINT(concurrency-mt-unsafe)
            }
            arguments->mac_arg = arg;
            break;
        case ARGP_KEY_END:
            if (state->arg_num < 1) {
                argp_usage(state);  // NOLINT(concurrency-mt-unsafe)
            }
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

const struct argp ARGP_PARSER = {OPTIONS.data(), parse_opt, ARGS_DOC, DOC, nullptr, nullptr, nullptr};

}  // namespace

int main(int argc, char** argv) {
    Arguments arguments{};
    argp_parse(&ARGP_PARSER, argc, argv, 0, nullptr, &arguments);  // NOLINT(concurrency-mt-unsafe)

    wol::AWakeOnLAN awakeonlan(arguments.ip_addr, arguments.port, arguments.mac_arg, arguments.passwd,
                               arguments.ipv6_bcast_toggle);
    awakeonlan.send_magic_packet();

    return 0;
}