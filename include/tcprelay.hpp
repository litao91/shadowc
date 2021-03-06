#ifndef SHADOW_C_TCPRELAY_H__
#define SHADOW_C_TCPRELAY_H__
#include "eventloop.hpp"
#include "asyncdns.hpp"

namespace tcprelay {
class TCPRelay;
class TCPRelayHandler;
typedef std::map<int, TCPRelayHandler*> fd_to_handlers_t;
class TCPRelayHandler {
    public:
        TCPRelayHandler(
                TCPRelay* tcp_relay,
                fd_to_handlers_t* fd_to_handlers,
                eventloop::EventLoop* loop,
                int local_sock, 
                asyncdns::DNSResolver* dns_resolver);
        ~TCPRelayHandler();
        void handle_event(const epoll_event* evt);
    private:
        TCPRelay* tcp_relay;
        int local_socket;
        asyncdns::DNSResolver* dns_resolver;
        fd_to_handlers_t* fd_to_handlers;
};



/**
 * TCPRelay, server only
 */
class TCPRelay: public eventloop::EventHandler {
    public:
        TCPRelay(asyncdns::DNSResolver* dns_resolver, const char* server, const char* server_port);
        virtual ~TCPRelay();
        void add_to_loop(eventloop::EventLoop* loop);
        virtual void handle_event(const epoll_event* evt);
    private:
        void sweep_timeout();
        int server_socket;
        eventloop::EventLoop* loop;
        bool closed;
        fd_to_handlers_t fd_to_handlers;
        asyncdns::DNSResolver* dns_resolver;
};

}
#endif
