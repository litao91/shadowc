#ifndef SHADOW_C_TCPRELAY_H__
#define SHADOW_C_TCPRELAY_H__
#include "eventloop.hpp"
#include "asyncdns.hpp"

namespace tcprelay {
class TCPRelayHandler: public eventloop::EventHandler {
    public:
        TCPRelayHandler();
        virtual ~TCPRelayHandler();
};


/**
 * TCPRelay, server only
 */
class TCPRelay: public eventloop::EventHandler {
    public:
        TCPRelay(asyncdns::DNSResolver* dns_resolver, const char* server, const char* server_port);
        void add_to_loop(eventloop::EventLoop* loop);
        void handle_event(const epoll_event* evt);
    private:
        int server_socket;
        eventloop::EventLoop* loop;
        bool closed;
};

}
#endif
