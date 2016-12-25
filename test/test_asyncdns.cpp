#include "asyncdns.hpp"
#include "eventloop.hpp"
#include <utility>
#include <string>
#include <iostream>

static int counter = 0;

void test_build_request() {
    std::pair<unsigned char*, int> req = asyncdns::build_request("www.google.com");
    delete [] req.first;
}


void test_callback(const std::string& ip, const std::string& hostname) {
    std::cout << "IP: " << ip << ", Host: " << hostname << std::endl;
    ++counter;
}

void test_send_dns() {
    asyncdns::DNSResolver resolver;
    eventloop::EventLoop loop;
    resolver.add_to_loop(&loop);
    resolver.resolve("google.com", test_callback);
    loop.run();
}

int main(int argc, char** argv) {
    test_send_dns();
}
