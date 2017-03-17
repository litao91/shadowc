#ifndef SHADOW_C_CRYPTOR__
#define SHADOW_C_CRYPTOR__
#include "crypto/crypto.hpp"

namespace cryptor {

class Cryptor {
    public:
        Cryptor(const char* password, const char* method);
        crypto::Crypto* get_cipher(
                const char* password, 
                const char* method, 
                int op);
        unsigned char* encrypt(const unsigned char* buf);
        unsigned char* decrypt(const unsigned char* buf);
};
}

#endif
