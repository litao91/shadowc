#include "tcprelay.hpp"
#include "utils.hpp"
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <iostream>

namespace tcprelay {
    //  as ssserver:
    //  stage 0 just jump to stage 1
    //  stage 1 addr received from local, query DNS for remote
    //  stage 3 DNS resolved, connect to remote
    //  stage 4 still connecting, more data from local received
    //  stage 5 remote connected, piping local and remote
    const static char STAGE_INIT = 0;
    const static char STAGE_ADDR = 1;
    const static char STAGE_UDP_ASSOC = 2;
    const static char STAGE_DNS = 3;
    const static char STAGE_CONNECTING = 4;
    const static char STAGE_STREAM = 5;
    const static char STAGE_DESTROYED = -1;

//  for each handler, we have 2 stream directions:
//     upstream:    from client to server direction
//                  read local and write to remote
//     downstream:  from server to client direction
//                  read remote and write to local

    const static char STREAM_UP = 0;
    const static char STREAM_DOWN = 1;

    // for each stream, it's waiting for reading, or writing, or both
    const static char WAIT_STATUS_INIT = 0;
    const static char WAIT_STATUS_READING = 1;
    const static char WAIT_STATUS_WRITING = 2;
    const static char WAIT_STATUS_READWRITING = WAIT_STATUS_READING | WAIT_STATUS_WRITING;

    const static int BUF_SIZE = 32 * 1024;


    // TCPRelay class
    TCPRelay::TCPRelay(asyncdns::DNSResolver* dns_resolver, const char* server, const char* server_port) {
        // init members
        loop = NULL;
        closed = false;

        // create server socket
        addrinfo hints;
        memset(&hints, 0, sizeof(addrinfo));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        
        addrinfo *result;
        int r;
        r = getaddrinfo(server, 
                server_port, 
                &hints,
                &result);
        if (r != 0) {
            std::cerr << "Get addr info error:" << gai_strerror(r) << std::endl;
            exit(1);
        }

        server_socket = 0;
        for (addrinfo* rp = result; rp != NULL; rp = rp->ai_next) {
            server_socket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            if (server_socket == -1) {
                continue;
            }
            int val = 1;
            r = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int));
            if (r < 0) {
                std::cerr << "setsockopt(SO_REUSEADDR) failed" << std::endl;
            }
            r = bind(server_socket, rp->ai_addr, rp->ai_addrlen);
            if (r == 0) {
                break;
            }
        }
        freeaddrinfo(result);
        utils::make_socket_non_blocking(server_socket);
        r = listen(server_socket, 1024);
        if (r == -1) {
            std::cerr << "Error listen" << std::endl;
            abort();
        }
    }

    void TCPRelay::add_to_loop(eventloop::EventLoop* loop) {
        if (this->loop != NULL) {
            std::cerr << "Loop already added" << std::endl;
            return;
        }
        if (this->closed) {
            std::cerr << "Already closed" << std::endl;
            return;
        }
        this->loop = loop;
        this->loop->add(this->server_socket, EPOLLIN | EPOLLERR, this);
    }

    void TCPRelay::handle_event(const epoll_event* evt) {
    }
}
