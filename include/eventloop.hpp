#ifndef SHADOW_C_EVENTLOOP_H__
#define SHADOW_C_EVENTLOOP_H__

#include<map>
#include<sys/epoll.h>

namespace eventloop {
    class EventHandler {
        public:
            virtual void handle_event(const epoll_event* evt) = 0;
    };

    class EventLoop {
        public:
            EventLoop();
            ~EventLoop();
            void add(int fd, int mode, EventHandler* handler);
            void remove(int fd);
            int run();
        private:
            int efd;
            std::map<int, EventHandler*> fd_handler_map;
    };
}
#endif
