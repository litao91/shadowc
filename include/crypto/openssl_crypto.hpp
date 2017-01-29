#ifndef SHADOW_C_OPENSSL_CRYPTO_H__
#define SHADOW_C_OPENSSL_CRYPTO_H__
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
namespace crypto {

class OpenSSLCrypto {
    public:
        OpenSSLCrypto(const char* cipher_name, 
                unsigned char *key, 
                unsigned char *iv, 
                int op);
        ~OpenSSLCrypto();
    private:
        EVP_CIPHER_CTX *cipher_ctx;
};

}

#endif
