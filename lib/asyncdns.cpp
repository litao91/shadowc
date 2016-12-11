#include "asyncdns.hpp"
#include "utils.cpp"
#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/udp.h>
#include <sys/types.h>
#include <netdb.h>

namespace asyncdns {
DNSResolver::DNSResolver() {
    std::cout << "Init DNSResolver" << std::endl;
    this->servers = new std::string[2];
    this->servers[0] = "8.8.4.4";
    this->servers[2] = "8.8.8.8";
    this->loop = NULL;
}

DNSResolver::~DNSResolver() {
    delete [] this->servers;
}

void DNSResolver::resolve(const std::string& hostname, callback_t cb) {
    std::cout << "Resolving: " << hostname << std::endl;
}

// Create the socket for dns resolver and add the socket to eventloop
void DNSResolver::add_to_loop(eventloop::EventLoop* loop) {
    if (this-> loop != NULL) {
        std::cerr << "Already have a loop" << std::endl;
        return;
    }
    this-> loop = loop;
    // create socket
    addrinfo hints;
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = SOL_UDP;
    addrinfo *result, *rp;
    int r = getaddrinfo(NULL, NULL, &hints, &result);
    if (r != 0) {
        std::cerr << "get addrinfo" << std::endl;
        return;
    }
    int sfd;
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue;
        r = utils::make_socket_non_bocking(sfd);
        if (r != 0) {
            std::cerr << "make non blocking" << std::endl;
            return;
        }
        this->dns_socket = sfd;
        break;
    }
    freeaddrinfo(result);
    // add to loop
    loop -> add(this->dns_socket, EPOLLIN, this);
}

void DNSResolver::handle_event(const epoll_event* evt) {
}
}
