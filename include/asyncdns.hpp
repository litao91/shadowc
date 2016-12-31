#ifndef SHADOW_C_ASYNCDNS_H__
#define SHADOW_C_ASYNCDNS_H__ 
#include <map>
#include <string>
#include <utility>
#include <vector>
#include "eventloop.hpp"
namespace asyncdns {
    void change_to_dns_name_format(unsigned char* dns, const unsigned char* host);

    // type for the asyncdns callback, function ptr 
    typedef void (*callback_t)(const std::string& ip, 
            const std::string& hostname);

    // the type from 
    typedef std::map<std::string, std::vector<callback_t>*> host_to_cb_t ;


    //DNS header structure
    struct DNS_HEADER
    {
        unsigned short id; // identification number

        unsigned char rd :1; // recursion desired
        unsigned char tc :1; // truncated message
        unsigned char aa :1; // authoritive answer
        unsigned char opcode :4; // purpose of message
        unsigned char qr :1; // query/response flag

        unsigned char rcode :4; // response code
        unsigned char cd :1; // checking disabled
        unsigned char ad :1; // authenticated data
        unsigned char z :1; // its z! reserved
        unsigned char ra :1; // recursion available

        unsigned short q_count; // number of question entries
        unsigned short ans_count; // number of answer entries
        unsigned short auth_count; // number of authority entries
        unsigned short add_count; // number of resource entries
    };

    //Constant sized fields of query structure
    struct QUESTION
    {
        unsigned short qtype;
        unsigned short qclass;
    };

    //Constant sized fields of the resource record structure
#pragma pack(push, 1)
    struct R_DATA
    {
        unsigned short type;
        unsigned short _class;
        unsigned int ttl;
        unsigned short data_len;
    };
#pragma pack(pop)

    //Pointers to resource record contents
    struct RES_RECORD
    {
        unsigned char *name;
        struct R_DATA *resource;
        unsigned char *rdata;
    };

    class DNSResolver: public eventloop::EventHandler {
        public:
            const int STATUS_FIRST = 0;
            const int STATUS_SECOND = 1;

            DNSResolver();
            virtual ~DNSResolver();
            void resolve(const char* hostname, callback_t cb);
            void add_to_loop(eventloop::EventLoop* loop);
            void create_dns_socket();
            void handle_event(const epoll_event* evt);
            void close();

            // for testing
            int get_dns_socket();
        private:
            const char** servers;
            int num_servers;
            eventloop::EventLoop* loop;
            int dns_socket;
            /*
             * Perform a DNS query by sending a packet
             */
            void send_req(const char* hostname, int query_type);
            host_to_cb_t hostname_to_cb;
            std::map<std::string, int> hostname_status_map;
    };
}
#endif
