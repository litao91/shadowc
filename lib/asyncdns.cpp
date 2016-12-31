#include "asyncdns.hpp"
#include "utils.hpp"
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

namespace asyncdns {
const static unsigned short QTYPE_ANY = 255;
const static unsigned short QTYPE_A = 1; // Ipv4 address
const static unsigned short QTYPE_AAAA = 28;
const static unsigned short QTYPE_CNAME = 5;


/**
 * This will convert www.google.com to 3www6google3com
 */
void change_to_dns_name_format(unsigned char* dns, const unsigned char* host) {
    std::cout << "Building request for " << host << std::endl;
    unsigned char label_len = 0;
    unsigned char* label_len_ptr = dns;
    dns++;
    while(*host != 0) {
        char v = *host;
        if (v == '.') {
            *label_len_ptr = label_len;
            label_len = 0;
            label_len_ptr = dns;
        } else {
            *dns = *host;
            ++label_len;
        }
        ++host;
        ++dns;
    }
    *label_len_ptr = label_len;
    *++dns='\0';
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
        this->send_req(hostname, QTYPE_A);
        this->hostname_to_cb[hostname]->push_back(cb);
    } else {
        this->hostname_to_cb[hostname]->push_back(cb);
        this->send_req(hostname, QTYPE_A);
    }
}

void DNSResolver::create_dns_socket() {
    if (this->dns_socket != 0) {
        std::cout << "Socket already created" << std::endl;
        return;
    }
    // create socket
    this->dns_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
}

// Create the socket for dns resolver and add the socket to eventloop
void DNSResolver::add_to_loop(eventloop::EventLoop* loop) {
    if (this-> loop != NULL) {
        std::cerr << "Already have a loop" << std::endl;
        return;
    }
    this->create_dns_socket();
    this->loop = loop;
    std::cout << "Creating socket" << std::endl;
    utils::make_socket_non_blocking(this->dns_socket);
    
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

void DNSResolver::send_req(const char* hostname, int query_type) {
    DNS_HEADER *dns = NULL;
    unsigned char buf[65536];

    // setup the header portion
    dns = (struct DNS_HEADER *)&buf;
    
    dns->id = (unsigned short) htons(getpid());
    dns->qr = 0; // This is query
    dns->opcode = 0; // This is 
    dns->aa = 0; // Not Authoritative
    dns->tc = 0; // This message is not truncated
    dns->rd = 1; // Recursion Desired
    dns->ra = 0; // Recursion not available
    dns->z = 0;
    dns->ad = 0;
    dns->cd = 0;
    dns->rcode = 0;
    dns->q_count = htons(1); // we have only 1 question
    dns->ans_count = 0;
    dns->auth_count = 0;
    dns->add_count = 0;

    // point to the query portion, append the host name
    unsigned char* qname = (unsigned char*)&buf[sizeof(struct DNS_HEADER)];
    change_to_dns_name_format(qname, (const unsigned char*) hostname);

    // Point to the start of question info
    QUESTION *qinfo = (QUESTION*)&buf[sizeof(DNS_HEADER) + (strlen((const char*)qname) + 1)];
    qinfo->qtype = htons(query_type); // type of the query, A, MX CNAME
    qinfo->qclass = htons(1);

    int len = sizeof(struct DNS_HEADER) + (strlen((const char*)qname)+1) + sizeof(struct QUESTION);
    std::cout << "Buf of size " << len << std::endl;
    for(int i = 0; i < num_servers; ++i) {
        const char* server = servers[i];
        std::cout << "Resolving " << hostname << "  using server " << server << std::endl;
        sockaddr_in sa;
        sa.sin_family = AF_INET;
        sa.sin_port = htons(53); // convert to network byteorder
        sa.sin_addr.s_addr = inet_addr(server);
        int r = sendto(this->dns_socket, (char *)buf, len, 0, (sockaddr *) &sa, sizeof(sa));
        std::cout << "Sent " << r << " bytes " << std::endl;
    }
}

int DNSResolver::get_dns_socket() {
    return this->dns_socket;
}

}
