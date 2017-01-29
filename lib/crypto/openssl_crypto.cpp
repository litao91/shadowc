#include "crypto/openssl_crypto.hpp"
#include "crypto/crypto.hpp"
#include <iostream>

namespace crypto {
    OpenSSLCrypto::OpenSSLCrypto(const char* cipher_name, 
            unsigned char* key, 
            unsigned char* iv, 
            int op) {
        // init the library
        ERR_load_crypto_strings();
        const EVP_CIPHER* cipher = EVP_get_cipherbyname(cipher_name);

        this->cipher_ctx = EVP_CIPHER_CTX_new();
        if (!this->cipher_ctx) {
            std::cerr << "Error creating new ctx context" << std::endl;
            return;
        }
        int r = EVP_CipherInit_ex(this->cipher_ctx, cipher, NULL, key, iv, op);
        if (!r) {
            std::cerr << "Error Init cipher" << std::endl;
            return;
        }
    }

    OpenSSLCrypto::~OpenSSLCrypto() {
        EVP_CIPHER_CTX_free(this->cipher_ctx);
    }
}

