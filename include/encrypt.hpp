#ifndef SHADOW_C_ENCRYPT__
#define SHADOW_C_ENCRYPT__
#include "crypto/crypto.hpp"

namespace encrypt {

class Encrypt {
    public:
        Encrypt(const char* password, const char* method);
        crypto::Crypto* get_cipher(
                const char* password, 
                const char* method, 
                unsigned char* iv,
                int op);
        unsigned char* encrypt(const unsigned char* buf);
        unsigned char* decrypt(const unsigned char* buf);
};
}

#endif
