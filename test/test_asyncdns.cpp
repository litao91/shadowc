#include "asyncdns.hpp"
#include <utility>
int main(int argc, char** argv) {
    std::pair<unsigned char*, int> req = asyncdns::build_request("www.google.com");
    delete [] req.first;
}
