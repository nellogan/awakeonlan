#include <argp.h>
#include <stdlib.h>

#include "a_wake_on_lan.h"


const char *argp_program_version = "awakeonlan 0.0.1";
const char *argp_program_bug_address = "<https://github.com/nellogan/awakeonlan/issues>";

static char doc[] =
"awakeonlan -- sends a WakeOnLAN packet to a MAC address on current Local Area Network or to an IPv4/IPv6 address over "
"the internet (UDP packet). By default, awakeonlan will broadcast the packet (255.255.255.255, port 9) if IP address "
"and port is not set. If the ipv6_mcast_toggle switch (-6) is activated with no IP address provided, the IPv6 multicast "
"address ff02::1 will be used. Normally a remote host on a LAN will only listen to broadcast/multicast packets when "
"in a sleep or unpowered state; Therefore, specify an IP address when waking over the internet (may need to configure "
"port forwarding on remote hosts network). Supports sending an optional SecureOn password. Supported MAC address "
"formats: XX:XX:XX:XX:XX:XX, XX-XX-XX-XX-XX-XX, XXXXXXXXXXXX (where XX is a valid hexadecimal value).";

static char args_doc[] = "MAC_ADDRESS";

static struct argp_option options[] = {
	{"ip_addr",             'i', "IP_ADDR",    0, "IP Address of machine of where the WakeOnLAN packet will be sent. "
	                                              "If unspecified, IP_ADDR=255.255.255.255", 0},
	{"port",                'p', "PORT",       0, "Port of machine of where the WakeOnLAN packet will be sent", 0},
	{"passwd",              'x', "PASSWD",     0, "SecureOn password, usually six hexadecimal bytes (12 ascii characters)", 0},
	{"ipv6_mcast_toggle",   '6', 0,            0, "If IP_ADDR is omitted, sets IP Address to the IPv6 multicast address "
	                                              "(ff02::1). This option is ignored if IP_ADDR is set.", 0},
	{ 0, 0, 0, 0, 0, 0 }
};

struct arguments
{
	char* args[1];
	char* ip_addr;
	char* port;
	char* passwd;
	bool ipv6_bcast_toggle;
};

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	struct arguments *arguments = static_cast<struct arguments*>(state->input);

	switch (key)
	{
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
			if (state->arg_num >= 1)
				// Too many arguments.
				argp_usage (state);
			arguments->args[state->arg_num] = arg;
			break;
		case ARGP_KEY_END:
			if (state->arg_num < 1)
				// Not enough arguments.
				argp_usage (state);
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

int main (int argc, char **argv)
{
	struct arguments arguments;
	arguments.ip_addr = nullptr;
	arguments.port = nullptr;
	arguments.passwd = nullptr;
	arguments.ipv6_bcast_toggle = false;
	argp_parse (&argp, argc, argv, 0, 0, &arguments);

    char* mac_arg = arguments.args[0];
    AWakeOnLAN a_wake_on_lan(arguments.ip_addr, arguments.port, mac_arg, arguments.passwd, arguments.ipv6_bcast_toggle);
    a_wake_on_lan.SendMagicPacket();

	return 0;
}