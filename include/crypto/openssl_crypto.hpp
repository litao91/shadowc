#ifndef SHADOW_C_OPENSSL_CRYPTO_H__
#define SHADOW_C_OPENSSL_CRYPTO_H__
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string>
#include "crypto.hpp"
namespace crypto {

class OpenSSLCrypto: public Crypto {
    public:
        OpenSSLCrypto(const std::string& cipher_name, 
                unsigned char *key, 
                unsigned char *iv, 
                int op);
        static const EVP_CIPHER* get_cipher_by_name(const std::string&);
        virtual ~OpenSSLCrypto();
        virtual void update(const unsigned char* data, int data_size, 
                unsigned char* out, int* out_size);
    private:
        EVP_CIPHER_CTX *cipher_ctx;
};

}

#endif
