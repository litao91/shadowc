#ifndef SOCKET_C_UTILS_H_
#define SOCKET_C_UTILS_H_
namespace utils {
    int make_socket_non_blocking(int sfd);
    void random_string(int length, char* buf);
}
#endif
