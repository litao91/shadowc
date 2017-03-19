#ifndef SHADOW_C_CIPHER_H__
#define SHADOW_C_CIPHER_H__
#include <stddef.h>

namespace crypto {
    class Crypto {
    public:
        virtual ~Crypto() = 0;
        virtual void update(const unsigned char* data, 
                int data_size, 
                unsigned char* out, int * out_size) = 0;
    };
}

#endif
