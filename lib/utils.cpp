#include "utils.hpp"
#include <unistd.h>
#include <iostream>
#include <fcntl.h>

namespace utils {
    int make_socket_non_blocking(int sfd) {
        int flags, s;

        flags = fcntl (sfd, F_GETFL, 0);
        if (flags == -1) {
            std::cerr << "fcntl" << std::endl;
            return -1;
        }

        flags |= O_NONBLOCK;
        s = fcntl (sfd, F_SETFL, flags);
        if (s == -1) {
            std::cerr << "fcntl" << std::endl;
            return -1;
        }

        return 0;
    }
}
