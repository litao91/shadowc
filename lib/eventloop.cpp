#include "eventloop.hpp"
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>
namespace eventloop {
EventLoop::EventLoop() {
    std::cout << "INIT eventloop" << std::endl;
    this->efd = epoll_create1(0); // todo error handling
}

EventLoop::~EventLoop() {
    close(this->efd);
}

void EventLoop::add(int fd, int mode, EventHandler* handler) {
    std::cout << "Addinig new socket " << fd << std::endl;
    epoll_event ev;
    ev.events = mode;
    if(epoll_ctl(this->efd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        std::cerr << "epoll_ctl_add" << std::endl;
    }
    this->fd_handler_map.insert(std::pair<int, EventHandler*>(fd, handler));
}

void EventLoop::remove(int fd) {
    epoll_ctl(this->efd, EPOLL_CTL_DEL, fd, NULL);
    this->fd_handler_map.erase(fd);
}

int EventLoop::run() {
    const int max_events = 64;
    epoll_event events[max_events];
    while(true) {
        int nfds = epoll_wait(this->efd, events, max_events, -1);
        if (nfds == -1) {
            if(errno != EINTR) {
                std::cerr << "epoll_wait" << std::endl;
                return -1;
            }
            continue;
        }
        std::cout << "Received " << nfds << " File descriptors";

        for (int n = 0; n < nfds; ++n) {
            epoll_event* e = events + n;
            int fd = e->data.fd;
            EventHandler* handler = this->fd_handler_map[fd];
            handler->handle_event(e);
        }
    }
}
}


