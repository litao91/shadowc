#ifndef SHADOW_C_ASYNCDNS_H__
#define SHADOW_C_ASYNCDNS_H__ 
#include<map>
#include<string>
#include "eventloop.hpp"
namespace asyncdns {
    typedef void (*callback_t)(const std::string& ip, 
            const std::string& hostname);

    class DNSResolver {
        public:
            DNSResolver();
            void resolve(const std::string& hostname, callback_t cb);
    };
}
#endif
