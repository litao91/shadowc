#ifndef SHADOW_C_CRYPTOR__
#define SHADOW_C_CRYPTOR__
#include "crypto/crypto.hpp"

namespace cryptor {

class Cryptor {
    public:
        Cryptor(const char* password, const char* method);
        ~Cryptor();
        crypto::Crypto* get_cipher(
                const char* password, 
                const char* method, 
                int op,
                unsigned char* iv);
        void encrypt(const unsigned char* buf, int len, 
                unsigned char* out, int* out_size);
        void decrypt(const unsigned char* buf, int len,
                unsigned char* out, int *out_size);
    private:
        inline void set_cipher_iv(unsigned char* iv);
        crypto::Crypto* cipher;
        crypto::Crypto* decipher;
        unsigned char* cipher_iv;
        unsigned char* decipher_iv;
        int cipher_iv_len;
        bool iv_sent;
        const char* method;
        const char* password;
};
}

#endif
