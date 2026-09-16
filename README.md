<div align="center">

# awakeonlan

[![CI-x86_64 Status](https://github.com/nellogan/awakeonlan/actions/workflows/CI-x86_64.yml/badge.svg)](https://github.com/nellogan/awakeonlan/actions/workflows/CI-x86_64.yml)
[![CI-aarch64 Status](https://github.com/nellogan/awakeonlan/actions/workflows/CI-aarch64.yml/badge.svg)](https://github.com/nellogan/awakeonlan/actions/workflows/CI-aarch64.yml)

</div>

`awakeonlan` is a Linux utility that sends a Wake-on-LAN packet to a MAC address on the current Local Area 
Network or to an IPv4/IPv6 address over the internet (via UDP packet). 

By default, `awakeonlan` will broadcast the packet (`255.255.255.255`, port `9`) if an IP address and port are not 
set. If the IPv6 multicast toggle switch (`-6`) is activated with no IP address provided, the IPv6 multicast address
`ff02::1` will be used. Normally a remote host on a LAN will only listen to broadcast/multicast packets when in a 
sleep or unpowered state; therefore, specify an IP address when waking over the internet (you may need to configure 
port forwarding on the remote host's network). It also supports sending an optional SecureOn password.

Supported MAC address formats: `XX:XX:XX:XX:XX:XX`, `XX-XX-XX-XX-XX-XX`, `XXXXXXXXXXXX` (where `XX` is a valid 
hexadecimal value).

## Install

```bash
git clone https://github.com/nellogan/awakeonlan.git
#Optionally run tests first: make test-sanitizers && make clean-all && make test-valgrind
make
make clean
sudo make install
#Optionally add to path via bashrc:
#echo 'export PATH="$PATH:/usr/local/awakeonlan/bin"' >> ~/.bashrc
```

## Uninstall

```bash
sudo make uninstall
#Do not forget to remove 'PATH="$PATH:/usr/local/awakeonlan/bin"' from ~/.bashrc
```

## Usage:

```text
    awakeonlan --help
    Usage: awakeonlan [OPTION...] MAC_ADDRESS
    awakeonlan -- sends a WakeOnLAN packet to a MAC address on current Local
    Area Network or to an IPv4/IPv6 address over the internet (UDP packet). By
    default, awakeonlan will broadcast the packet (255.255.255.255, port 9) if IP
    address and port is not set. If the ipv6_mcast_toggle switch (-6) is activated
    with no IP address provided, the IPv6 multicast address ff02::1 will be used.
    Normally a remote host on a LAN will only listen to broadcast/multicast packets
    when in a sleep or unpowered state; Therefore, specify an IP address when
    waking over the internet (may need to configure port forwarding on remote hosts
    network). Supports sending an optional SecureOn password. Supported MAC address
    formats: XX:XX:XX:XX:XX:XX, XX-XX-XX-XX-XX-XX, XXXXXXXXXXXX (where XX is a
    valid hexadecimal value).
    
      -6, --ipv6_mcast_toggle    If IP_ADDR is omitted, sets IP Address to the IPv6
                                 multicast address (ff02::1). This option is
                                 ignored if IP_ADDR is set.
      -i, --ip_addr=IP_ADDR      IP Address of machine of where the WakeOnLAN
                                 packet will be sent. If unspecified,
                                 IP_ADDR=255.255.255.255
      -p, --port=PORT            Port of machine of where the WakeOnLAN packet will
                                 be sent
      -x, --passwd=PASSWD        SecureOn password, usually six hexadecimal bytes
                                 (12 ascii characters)
      -?, --help                 Give this help list
          --usage                Give a short usage message
      -V, --version              Print program version
    
    Mandatory or optional arguments to long options are also mandatory or optional
    for any corresponding short options.
    
    Report bugs to <https://github.com/nellogan/awakeonlan/issues>.
```

## Examples
Note: if not added to path replace 'awakeonlan' with './bin/awakeonlan'.

### Wake up host on the same Local Area Network:

```bash
    awakeonlan AB:CD:EF:12:34:56
```

### Wake up host on the same LAN using an IPv6 multicast instead of the default IPv4 broadcast:

```bash
    awakeonlan -6 AB:CD:EF:12:34:56
```

### Wake up remote host by specifying an IPv4 address, port, and SecureOn password:
Here Google's public DNS IPv4 address is used.

```bash
    awakeonlan -i 8.8.8.8 -p 63000 -x passwd AB:CD:EF:12:34:56
```

### Wake up remote host by specifying an IPv6 address, port, and SecureOn password:
Here Google's public DNS IPv6 address is used.

```bash
    awakeonlan -i 2001:4860:4860::8888 -p 63000 -x passwd AB:CD:EF:12:34:56
```

## Testing

### Native Local Testing

Format and lint the source:

```bash
make format
make lint
```

Run the test suite with AddressSanitizer, LeakSanitizer, and UndefinedBehaviorSanitizer:

```bash
make CONFIG=xsan test
```

Run the test suite with MemorySanitizer:

```bash
make CONFIG=msan test
```

Run the test suite under Valgrind:

```bash
make test-valgrind
```

### Containerized Testing

Run the containerized test suite locally:

```bash
bash ./test-container.sh
```

The containerized tests cover native x86_64 execution as well as AArch64 execution through emulation.
