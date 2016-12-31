#include "asyncdns.hpp"
#include "eventloop.hpp"
#include <utility>
#include <string>
#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/udp.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <vector>
#include <utility>
#include <unistd.h> // getpid
#include <string.h>

static int counter = 0;

void test_build_request() {
    unsigned char buf[65536];
    const char* host = "www.google.com";
    asyncdns::change_to_dns_name_format(buf, (const unsigned char*)host);
    unsigned char* iter = buf;
    while (*iter != 0) {
        std::cout << int(*iter++) << "," << std::endl;
    }
}

void test_callback(const std::string& ip, const std::string& hostname) {
    std::cout << "IP: " << ip << ", Host: " << hostname << std::endl;
    ++counter;
}

void test_dns_resolve_without_loop() {
    eventloop::EventLoop loop;
    asyncdns::DNSResolver resolver;
    resolver.create_dns_socket();
    const char* host = "www.google.com";
    resolver.resolve(host, test_callback);
    int s = resolver.get_dns_socket();
    unsigned char buf[65536];

    struct sockaddr_in dest;
    int i = sizeof dest;
    dest.sin_family = AF_INET;
    dest.sin_port = htons(53);
    int r = recvfrom(s, (char*)buf, 65536, 0, (struct sockaddr*)&dest, (socklen_t*)&i);
    if (r < 0) {
        std::cerr << "recvfrom failed: " << r << std::endl;
    } else {
        std::cout << "recvfrom returns " << r << std::endl;
    }
}

void test_send_dns() {
    asyncdns::DNSResolver resolver;
    eventloop::EventLoop loop;
    resolver.add_to_loop(&loop);
    resolver.resolve("www.google.com", test_callback);
    loop.run();
}

int main(int argc, char** argv) {
    test_dns_resolve_without_loop();
}
