#ifndef SHADOW_C_ASYNCDNS_H__
#define SHADOW_C_ASYNCDNS_H__ 
#include<map>
#include<string>
#include "eventloop.hpp"
namespace asyncdns {
    typedef void (*callback_t)(const std::string& ip, 
            const std::string& hostname);

    class DNSResolver: public eventloop::EventHandler {
        public:
            DNSResolver();
            virtual ~DNSResolver();
            void resolve(const char* hostname, callback_t cb);
            void add_to_loop(eventloop::EventLoop* loop);
            void handle_event(const epoll_event* evt);
        private:
            std::string* servers;
            eventloop::EventLoop* loop;
            int dns_socket;
            void send_req(const char* hostname);
    };
}
#endif
