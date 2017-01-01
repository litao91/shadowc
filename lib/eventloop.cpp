#include "eventloop.hpp"
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>
namespace eventloop {
EventLoop::EventLoop() {
    std::cout << "INIT eventloop" << std::endl;
    this->efd = epoll_create1(0); // todo error handling
    if (this->efd == -1) {
        std::cerr << "Error creating epoll fd" << std::endl;
        exit(1);
    }
    std::cout << "Created epoll fd " << this->efd << std::endl;
}

EventLoop::~EventLoop() {
    close(this->efd);
}

void EventLoop::add(int fd, int mode, EventHandler* handler) {
    std::cout << "Addinig new socket " << fd << " to event loop" <<  std::endl;
    epoll_event ev;
    ev.events = mode;
    ev.data.fd = fd;
    if(epoll_ctl(this->efd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        std::cerr << "Error epoll_ctl_add" << std::endl;
    }
    this->fd_handler_map.insert(std::pair<int, EventHandler*>(fd, handler));
}

void EventLoop::remove(int fd) {
    epoll_ctl(this->efd, EPOLL_CTL_DEL, fd, NULL);
    this->fd_handler_map.erase(fd);
}

int EventLoop::run() {
    std::cout << "Event loop starting..." << std::endl;
    const int max_events = 64;
    epoll_event events[max_events];
    while(true) {
        std::cout << "Waiting for epoll" << std::endl;
        int nfds = epoll_wait(this->efd, events, max_events, -1);
        std::cout << "Epoll wait returned" << std::endl;
        if (nfds == -1) {
            if(errno != EINTR) {
                std::cerr << "Error on epoll_wait" << std::endl;
                return -1;
            }
            continue;
        }
        std::cout << "Received " << nfds << " File descriptors";

        for (int n = 0; n < nfds; ++n) {
            epoll_event* e = events + n;
            if ((e->events & EPOLLERR) ||
                (e->events & EPOLLHUP) ||
                !(e->events & EPOLLIN))
            {
                std::cerr << "Epoll error " << std::endl;
            } else {
                int fd = e->data.fd;
                std::cout << "Retrieved fd " << fd << std::endl;
                auto handler_it = this->fd_handler_map.find(fd);
                if (handler_it == this->fd_handler_map.end()) {
                    std::cerr << "Can't find handler for fd: " << fd << std::endl;
                    continue;
                }
                handler_it->second->handle_event(e);
            }
        }
    }
}
}
