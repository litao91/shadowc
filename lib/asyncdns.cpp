#include "asyncdns.hpp"
#include "utils.cpp"
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

namespace asyncdns {
const static unsigned short QTYPE_ANY = 255;
const static unsigned short QTYPE_A = 1;
const static unsigned short QTYPE_AAAA = 28;
const static unsigned short QTYPE_CNAME = 5;

std::pair<unsigned char*, int> build_request(const char* address) {
    std::vector<unsigned char> buf;
    srand(time(NULL));
    unsigned short rnd = rand();
    // build request, note that network byte order is big-endian
    // request id
    buf.push_back(rnd >> 8);
    buf.push_back(rnd & 0xFF);
    // header (10 bytes)
    buf.push_back(1);
    buf.push_back(0);
    buf.push_back(0);
    buf.push_back(1);
    buf.push_back(0);
    buf.push_back(0);
    buf.push_back(0);
    buf.push_back(0);
    buf.push_back(0);
    buf.push_back(0);

    // build address, TODO: error handling
    int label_len_idx = buf.size();
    buf.push_back(0); // just a place holder
    unsigned char label_len = 0;
    while(*address != 0) {
        char v = *address;
        if (v == '.') {
            buf[label_len_idx] = label_len;
            label_len = 0;
            label_len_idx = buf.size();
            buf.push_back(0); // place holder for the length of next label
        } else {
            buf.push_back(v);
            ++label_len;
        }
        ++address;
    }
    buf[label_len_idx] = label_len;
    buf.push_back(0);
    // intel is little endian, we don't care about other 
    // QTYPE_A
    buf.push_back(QTYPE_A >> 8); // most significant bit
    buf.push_back(QTYPE_A & 0xFF);

    // QCLASS_IN=1
    buf.push_back(0);
    buf.push_back(1);

    unsigned char* req = new unsigned char[buf.size()];
    std::cout << "Request: ";
    for (size_t i = 0; i < buf.size(); ++i) {
        unsigned char c = buf[i];
        std::cout << i << ":" << c;
        req[i] = buf[i];
    }
    std::cout << std::endl;
    return std::pair<unsigned char*, int>(req, buf.size());
}

DNSResolver::DNSResolver() {
    std::cout << "Init DNSResolver" << std::endl;
    // TODO: pass domain servers as arguments
    this->num_servers = 2;
    this->servers = new const char*[num_servers];
    this->servers[0] = "8.8.4.4";
    this->servers[1] = "8.8.8.8";
    this->loop = NULL;
    this->dns_socket = 0;
}

DNSResolver::~DNSResolver() {
    delete [] this->servers;
    host_to_cb_t::iterator it = hostname_to_cb.begin();
    while(it != hostname_to_cb.end()) {
        delete it->second;
        ++it;
    }
}

void DNSResolver::close() {
    if (this->dns_socket) {
        ::close(this->dns_socket);
        if (this->loop) {
            this->loop->remove(this->dns_socket);
        }
    }

}

void DNSResolver::resolve(const char* hostname, callback_t cb) {
    std::cout << "Resolving host name: " << hostname << std::endl;
    // TODO: Cache and hostname sanity check
    // Add the call back to hostname_to_cb
    host_to_cb_t::iterator it = this->hostname_to_cb.find(hostname);
    if (it == hostname_to_cb.end()) {
        // add new entry for the hostname
        hostname_to_cb[hostname] = new std::vector<callback_t>();
        hostname_status_map[hostname] = this->STATUS_FIRST;
        this->send_req(hostname);
        this->hostname_to_cb[hostname]->push_back(cb);
    } else {
        this->hostname_to_cb[hostname]->push_back(cb);
        this->send_req(hostname);
    }
}

// Create the socket for dns resolver and add the socket to eventloop
void DNSResolver::add_to_loop(eventloop::EventLoop* loop) {
    if (this-> loop != NULL) {
        std::cerr << "Already have a loop" << std::endl;
        return;
    }
    this-> loop = loop;
    std::cout << "Creating socket" << std::endl;
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
    // add to loop, wait for incoming data, edge trigger
    loop -> add(this->dns_socket, EPOLLIN | EPOLLET, this);
}

void DNSResolver::handle_event(const epoll_event* evt) {
    int fd = evt->data.fd;
    if (this->dns_socket != fd) {
        return;
    }

    if (evt->events & EPOLLERR) {
        std::cerr << "epoll error" << std::endl;
        ::close(fd);
        return;
        // TODO: create new socket and start over again
    } else {
        char buff[1024];
        sockaddr src_addr;
        socklen_t peer_addr_len = sizeof(sockaddr_storage);
        ssize_t nread = recvfrom(fd, buff, 1024, 0, &src_addr, &peer_addr_len);
        std::cout << "Read " << nread << " bytes" << std::endl;
        char *ip = inet_ntoa(((sockaddr_in*) &src_addr) -> sin_addr);
        std::cout << "Resolve dns from "  << ip << std::endl;
    }
}

void DNSResolver::send_req(const char* hostname) {
    std::pair<unsigned char*, int> req = build_request(hostname);
    unsigned char* buf = req.first;
    size_t len = req.second;

    for(int i = 0; i < num_servers; ++i) {
        const char* server = servers[i];
        std::cout << "Resolving " << hostname << "  using server " << server << std::endl;
        sockaddr_in sa;
        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = inet_addr(server);
        sa.sin_port = 53;
        sendto(this->dns_socket, buf, len, 0, (sockaddr *) &sa, sizeof(sa));
    }

    delete [] buf;
}
}
