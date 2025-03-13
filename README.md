## Install
    git clone https://github.com/nellogan/awakeonlan.git
    #Optionally run tests first: make test-sanitizers && make clean-all && make test-valgrind
    make
    make clean
    sudo make install
    #Optionally add to path via bashrc:
    #echo 'export PATH="$PATH:/usr/local/awakeonlan/bin"' >> ~/.bashrc

## Uninstall
    sudo make uninstall
    #Do not forget to remove 'PATH="$PATH:/usr/local/awakeonlan/bin"' from ~/.bashrc

## Examples
Note: if not added to path replace 'awakeonlan' with './bin/awakeonlan'.

### Get help:

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


### Wake up host on the same Local Area Network:

    awakeonlan AB:CD:EF:12:34:56

### Wake up host on the same LAN using an IPv6 multicast instead of the default IPv4 broadcast:
    
    awakeonlan -6 AB:CD:EF:12:34:56

### Wake up remote host by specifying an IPv4 address, port, and SecureOn password:
Here Google's public DNS IPv4 address is used.

    awakeonlan -i 8.8.8.8 -p 63000 -x passwd AB:CD:EF:12:34:56

### Wake up remote host by specifying an IPv6 address, port, and SecureOn password:
Here Google's public DNS IPv6 address is used.

    awakeonlan -i 2001:4860:4860::8888 -p 63000 -x passwd AB:CD:EF:12:34:56
