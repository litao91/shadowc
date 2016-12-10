#include "eventloop.hpp"
#include <iostream>
#include <sys/epoll.h>
namespace eventloop {
EventLoop::EventLoop() {
    std::cout << "INIT eventloop" << std::endl;
    this->efd = epoll_create1(0); // todo error handling
}

EventLoop::~EventLoop() {
    close(this->efd);
}

void EventLoop::add(int fd, int mode) {
    epoll_event ev;
    ev.events = EPOLLIN | EPOLLLRDHUP | EPOLLET; 
    if(epoll_ctl(this->efd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        std::cerr << "epoll_ctl_add" << std::endl;
    }
}
