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
    char buf[65536];

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
    auto result = asyncdns::parse_response(buf);
    asyncdns::RES_RECORD* ans = result.first;
    int ans_len = result.second;

    sockaddr_in a;
    for (int i = 0; i < ans_len; ++i) {
        asyncdns::RES_RECORD* rec = ans + i;
        std::cout << "Name: " << rec->name << " ";
        int type = ntohs(rec->resource->type);
        if (type == 1) {
            long* p;
            p = (long*) rec->rdata;
            a.sin_addr.s_addr=(*p);
            std::cout << "has IPv4 address: " << inet_ntoa(a.sin_addr) << std::endl;
        }

        if (type == 5) {
            std::cout << "has alias name: " << rec->rdata << std::endl;
        }
    }
    free_res_record(result);
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
