#ifndef SHADOW_C_EVENTLOOP_H__
#define SHADOW_C_EVENTLOOP_H__
#define MAXEVENTS 64

namespace eventloop {
    class EventLoop {
        public:
            EventLoop();
            ~EventLoop();
            void poll(int timeout);
            void add(int fd, int mode);
            void run();
        private:
            int efd;
    };
}
#endif
