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
#include <tuple>

namespace asyncdns {
const static unsigned short QTYPE_ANY = 255;
const static unsigned short QTYPE_A = 1; // Ipv4 address
const static unsigned short QTYPE_AAAA = 28;
const static unsigned short QTYPE_CNAME = 5;

u_char* read_name(const unsigned char* reader, const unsigned char* buffer, int* count) {
    unsigned char *name;
    unsigned int p=0,jumped=0,offset;
    int i , j;

    *count = 1;
    name = (unsigned char*)malloc(256);

    name[0]='\0';

    //read the names in 3www6google3com format
    while(*reader!=0)
    {
        if(*reader>=192)
        {
            offset = (*reader)*256 + *(reader+1) - 49152; //49152 = 11000000 00000000 ;)
            reader = buffer + offset - 1;
            jumped = 1; //we have jumped to another location so counting wont go up!
        }
        else
        {
            name[p++]=*reader;
        }

        reader = reader+1;

        if(jumped==0)
        {
            *count = *count + 1; //if we havent jumped to another location then we can count up
        }
    }

    name[p]='\0'; //string complete
    if(jumped==1)
    {
        *count = *count + 1; //number of steps we actually moved forward in the packet
    }

    //now convert 3www6google3com0 to www.google.com
    for(i=0;i<(int)strlen((const char*)name);i++) 
    {
        p=name[i];
        for(j=0;j<(int)p;j++) 
        {
            name[i]=name[i+1];
            i=i+1;
        }
        name[i]='.';
    }
    name[i-1]='\0'; //remove the last dot
    return name;
}

std::pair<RES_RECORD*, int> parse_response(const char* data) {
    DNS_HEADER *dns = (DNS_HEADER*) data;
    std::cout << "The response contains " << std::endl;
    std::cout << "\t" << ntohs(dns->q_count) << " Questions" << std::endl;
    std::cout << "\t" << ntohs(dns->ans_count) << " Answers" << std::endl;
    std::cout << "\t" << ntohs(dns->auth_count) << " Authoritative Servers."<< std::endl;
    std::cout << "\t" << ntohs(dns->add_count) << " Additional records\n" << std::endl;


    // prepare reader
    size_t header_len = sizeof(DNS_HEADER);
    size_t query_len = strlen(data + header_len) + 1;
    size_t question_len = sizeof(QUESTION);
    size_t reader_start_idx = header_len + query_len + question_len;
    std::cout << "Reader starts at " << reader_start_idx << std::endl;
    const unsigned char* reader = (const unsigned char*) &data[reader_start_idx];

    // Start reading answers
    int stop = 0;

    int ans_count = ntohs(dns->ans_count);
    RES_RECORD* answers = (RES_RECORD*)malloc(sizeof(RES_RECORD)*ans_count);

    for (int i = 0; i < ans_count; ++i) {
        answers[i].name = read_name(reader, (const unsigned char*) data, &stop);
        std::cout << "received name " << answers[i].name << std::endl;
        reader = reader + stop;
        answers[i].resource = (R_DATA*)(reader);
        reader = reader + sizeof(R_DATA);
        if (ntohs(answers[i].resource->type) == 1) { // if ipv4
            std::cout << "IP v4 received" << std::endl;
            int resource_len = ntohs(answers[i].resource->data_len);
            answers[i].rdata = (unsigned char*)malloc(resource_len);
            for (int j = 0; j < resource_len; ++j) {
                answers[i].rdata[j] = reader[j];
            }
            answers[i].rdata[resource_len] = '\0';
        } else {
            answers[i].rdata = read_name(reader, (const unsigned char*) data, &stop);
            reader = reader + stop;
        }
        std::cout << "rdata " << answers[i].rdata << std::endl;
    }
    return std::pair<RES_RECORD*, int>(answers, ntohs(dns->ans_count));
}


void free_res_record(std::pair<RES_RECORD*, int> records) {
    for(int i = 0; i < records.second; ++i) {
        free((records.first + i)->name);
        free((records.first + i)->rdata);
    }
    free(records.first);
}

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
    unsigned char buf[128];

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
