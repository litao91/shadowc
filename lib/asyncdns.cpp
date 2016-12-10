#include "asyncdns.hpp"
#include<iostream>
#include<string>

namespace asyncdns {
DNSResolver::DNSResolver() {
    std::cout << "Init DNSResolver" << std::endl;
}

void DNSResolver::resolve(const std::string& hostname, callback_t cb) {
    std::cout << "Resolving: " << hostname << std::endl;
}
}
