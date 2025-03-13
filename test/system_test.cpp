#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
#include <cstring>
#include <cassert>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "utils.h"
#include "a_wake_on_lan.h"


void MockMachine(
    std::mutex& mtx,
    std::condition_variable& cv,
    bool& data_ready,
    char* ip_addr_arg,
    char* port_arg,
    char* mac_arg,
    char* passwd_arg
)
{
    int port = port_arg == nullptr ? DEFAULT_PORT : static_cast<int>(std::stol(port_arg));

    char buf[INET6_ADDRSTRLEN];
    int domain = -1;
    if (inet_pton(AF_INET, ip_addr_arg, buf))
    {
        domain = AF_INET;
    }
    else if (inet_pton(AF_INET6, ip_addr_arg, buf))
    {
        domain = AF_INET6;
    }
    else
    {
        throw std::invalid_argument("Error: MockMachine, invalid ip_addr_arg.\n");
    }

    int socket_fd = socket(domain, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_fd < 0)
    {
        std::error_code ec(errno, std::system_category());
        throw std::system_error(ec, "Error: MockMachine, failed to create socket.\n");
    }

    struct sockaddr_in server_sockaddr;
    struct sockaddr_in6 server_sockaddr_v6;

    int bind_err_code = -1;
    if (domain == AF_INET)
    {
        server_sockaddr.sin_family = domain;
        server_sockaddr.sin_port = htons(port);
        server_sockaddr.sin_addr.s_addr = INADDR_ANY;
        bind_err_code = bind(socket_fd, (struct sockaddr *)&server_sockaddr, sizeof(server_sockaddr));
    }
    else
    {
        std::memset(&server_sockaddr_v6, 0, sizeof(struct sockaddr_in6));
        server_sockaddr_v6.sin6_family = AF_INET6;
        server_sockaddr_v6.sin6_port = htons(port);
        server_sockaddr_v6.sin6_addr = in6addr_any;
        bind_err_code = bind(socket_fd, (struct sockaddr *)&server_sockaddr_v6, sizeof(server_sockaddr_v6));
    }

    if (bind_err_code < 0)
    {
        close(socket_fd);
        std::error_code ec(errno, std::system_category());
        throw std::system_error(ec, "Error: MockMachine, failed to bind socket.\n");
    }

    { // unique_lock will be destroyed after end of {} scope.
        std::unique_lock<std::mutex> lock(mtx);
        data_ready = true;
        cv.notify_one();
        lock.unlock();
    }

    unsigned char buffer[1024];
    int bytes_recvd = -1;
    unsigned int sock_addr_len = 0;
    if (domain == AF_INET)
    {
        sock_addr_len = sizeof(struct sockaddr_in);
        bytes_recvd = recvfrom(socket_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&server_sockaddr, &sock_addr_len);
    }
    else
    {
        sock_addr_len = sizeof(struct sockaddr_in6);
        bytes_recvd = recvfrom(socket_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&server_sockaddr_v6, &sock_addr_len);
    }

    if (bytes_recvd < 0)
    {
        close(socket_fd);
        std::error_code ec(errno, std::system_category());
        throw std::system_error(ec, "Error: MockMachine, failed to receive message.\n");
    }
    close(socket_fd);
    buffer[bytes_recvd] = '\0';


    std::string check_string = std::string("ffffffffffff");
    if (mac_arg[2] == ':' || mac_arg[2] == '-')
    {
        SanitizeMACAddr(mac_arg);
    }
    for (int i = 0; i<16; i++)
    {
        check_string.append(mac_arg, mac_arg+12);
    }
    std::vector<uint8_t> check_vec;
    HexStringToBytesVec(check_string, check_vec);
    if (passwd_arg)
    {
        check_vec.insert(check_vec.end(), passwd_arg, passwd_arg+std::strlen(passwd_arg));
    }

//    for (int i=0; i<bytes_recvd; i++)
//    {
//        std::printf("i %d buf[i] %02x check_vec[i] %02x\n", i, buffer[i], check_vec[i]);
//    }
    assert( std::memcmp(buffer, check_vec.data(), bytes_recvd*sizeof(uint8_t)) == 0 &&
            "Error: MockMachine, validation of final result failed.\n");
}

void MockAWakeOnLAN(
    std::mutex& mtx,
    std::condition_variable& cv,
    bool& data_ready,
    char* ip_addr_arg,
    char* port_arg,
    char* mac_arg,
    char* passwd_arg,
    bool ipv6_toggle_arg
)
{
    std::unique_lock<std::mutex> lock(mtx);
    bool mock_machine_ready = cv.wait_for(lock, std::chrono::seconds(3), [&](){ return data_ready; });
    if (!mock_machine_ready)
    {
        throw std::invalid_argument("Error: MockAWakeOnLAN, MockMachine unresponsive.\n");
    }
    lock.unlock();

    AWakeOnLAN a_wake_on_lan(ip_addr_arg, port_arg, mac_arg, passwd_arg, ipv6_toggle_arg);
    a_wake_on_lan.SendMagicPacket();
}

void TestHandler(char* ip_addr_arg, char* port_arg, char* mac_arg, char* passwd_arg, bool ipv6_toggle_arg)
{
    std::mutex mtx;
    std::condition_variable cv;
    bool data_ready = false;

    // AWakeOnLAN modifies mac_arg which is a potential data race.
    char mac_arg_copy[18] = { 0 };
    std::memcpy(mac_arg_copy, mac_arg, std::strlen(mac_arg));
    std::thread receiver(
        MockMachine,
        std::ref(mtx),
        std::ref(cv),
        std::ref(data_ready),
        ip_addr_arg,
        port_arg,
        mac_arg_copy,
        passwd_arg
    );
    std::thread sender(
        MockAWakeOnLAN,
        std::ref(mtx),
        std::ref(cv),
        std::ref(data_ready),
        ip_addr_arg,
        port_arg,
        mac_arg,
        passwd_arg,
        ipv6_toggle_arg
    );
    receiver.join();
    sender.join();
}

void SystemTest1()
{
    char ip_addr_arg[] = "127.0.0.1"; // ipv4 loopback addr range [127.0.0.1 - 127.254.255.255]
    char *port_arg = nullptr;
    char mac_arg[] = "AB:22:CD:FF:33:77";
    char *passwd_arg = nullptr;
    bool ipv6_toggle_arg = false;
    TestHandler(ip_addr_arg, port_arg, mac_arg, passwd_arg, ipv6_toggle_arg);
}

void SystemTest2()
{
    char ip_addr_arg[] = "127.254.255.255"; // ipv4 loopback addr range [127.0.0.1 - 127.254.255.255]
    char port_arg[] = "62000";
    char mac_arg[] = "CC-d2-3D-22-99-Aa";
    char passwd_arg[] = "passwd";
    bool ipv6_toggle_arg = false;
    TestHandler(ip_addr_arg, port_arg, mac_arg, passwd_arg, ipv6_toggle_arg);
}

void SystemTest3()
{
    char ip_addr_arg[] = "::1"; // ipv6 loopback addr
    char port_arg[] = "63000";
    char mac_arg[] = "abcdEf123456";
    char passwd_arg[] = "passwd";
    bool ipv6_toggle_arg = false;
    TestHandler(ip_addr_arg, port_arg, mac_arg, passwd_arg, ipv6_toggle_arg);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////Use the commented out functions below for troubleshooting on a live network.//////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//const char TARGET_MACHINE_IP[] = "XXX.XXX.XXX.XXX"; // Or desired ipv6 addr.
//const char TARGET_MAC_ADDR[] = "XX:XX:XX:XX:XX:XX";
char TARGET_MAC_ADDR[] = "E0-D5-5E-6B-84-3B";

////Send magic packet to potentially remote IPv4 addr.
//void IPV4Test()
//{
//    char ip_addr_arg[] = TARGET_MACHINE_IP; // ipv4 loopback addr range [127.0.0.1 - 127.254.255.255]
//    char port_arg[] = "9";
//    char mac_arg[] = TARGET_MAC_ADDR;
//    char* passwd_arg = nullptr;
//    bool ipv6_toggle_arg = false;
//    AWakeOnLAN a_wake_on_lan(ip_addr_arg, port_arg, mac_arg, passwd_arg, ipv6_toggle_arg);
//    a_wake_on_lan.SendMagicPacket();
//}
//
//void IPV4BroadcastTest()
//{
//    char* ip_addr_arg = nullptr;
//    char* port_arg = nullptr;
//    char mac_arg[] = TARGET_MAC_ADDR;
//    char* passwd_arg = nullptr;
//    bool ipv6_toggle_arg = false;
//    AWakeOnLAN a_wake_on_lan(ip_addr_arg, port_arg, mac_arg, passwd_arg, ipv6_toggle_arg);
//    a_wake_on_lan.SendMagicPacket();
//}
//
//void IPV6MulticastTest()
//{
//    char ip_addr_arg[] = "ff02::1"; // ipv6 loopback
//    char port_arg[] = "60000";
//    char mac_arg[] = TARGET_MAC_ADDR;
//    char* passwd_arg = nullptr;
//    bool ipv6_toggle_arg = false;
//    AWakeOnLAN a_wake_on_lan(ip_addr_arg, port_arg, mac_arg, passwd_arg, ipv6_toggle_arg);
//    a_wake_on_lan.SendMagicPacket();
//}
//
void IPV6MulticastToggleTest()
{
    char* ip_addr_arg = nullptr;
    char port_arg[] = "61234";
    char *mac_arg = TARGET_MAC_ADDR;
    char* passwd_arg = nullptr;
    bool ipv6_toggle_arg = true;
    AWakeOnLAN a_wake_on_lan(ip_addr_arg, port_arg, mac_arg, passwd_arg, ipv6_toggle_arg);
    a_wake_on_lan.SendMagicPacket();
}

int main()
{
    // NOTE: SystemTest1 uses DEFAULT_PORT(9) which is a privileged port and requires root or superuser permissions.
//    SystemTest1();
    SystemTest2();
    // NOTE: SystemTest3 uses the ipv6 loop back address "::01"; therefore, the system running this test
    // requires IP version 6 support.
    SystemTest3();

//    IPV4Test();
//    IPV4BroadcastTest();
//    IPV6MulticastTest();
//    IPV6MulticastToggleTest();
    return 0;
}