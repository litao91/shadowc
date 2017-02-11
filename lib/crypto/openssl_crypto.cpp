#include "crypto/openssl_crypto.hpp"
#include "crypto/crypto.hpp"
#include <openssl/aes.h>
#include <openssl/des.h>
#include <iostream>

namespace crypto {

    OpenSSLCrypto::OpenSSLCrypto(const char* cipher_name, 
            unsigned char* key, 
            unsigned char* iv, 
            int op) {
        std::cout << "INIT OpenSSLCrypto" << std::endl;
        // init the library
        const EVP_CIPHER* cipher = EVP_aes_256_cfb(); // EVP_get_cipherbyname(cipher_name);
        if (cipher == NULL) {
            std::cerr << "Can't find cipher " << cipher_name << std::endl;
            return;
        }

        this->cipher_ctx = NULL;
        this->cipher_ctx = EVP_CIPHER_CTX_new();
        if (this->cipher_ctx == NULL) {
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
        if (this->cipher_ctx != NULL) {
            EVP_CIPHER_CTX_cleanup(this->cipher_ctx);
            EVP_CIPHER_CTX_free(this->cipher_ctx);
        }
    }

    void OpenSSLCrypto::update(const unsigned char* data, int data_size, 
            unsigned char* out, int* out_size) {
        if(!EVP_EncryptUpdate(this->cipher_ctx, out, out_size, data, data_size)) {
            std::cerr << "Error update encryption" << std::endl;
            return;
        }
    }
}

