#include "crypto/openssl_crypto.hpp"
#include "crypto/crypto.hpp"
#include <openssl/aes.h>
#include <openssl/des.h>
#include <openssl/evp.h>
#include <iostream>
#include <map>
#include <string>

namespace crypto {
    typedef const EVP_CIPHER* (* cipher_ctor_t) (void);
    inline Crypto::~Crypto() {}

    
    static std::map<std::string, cipher_ctor_t> create_name_cipher_map() {
        std::map<std::string, cipher_ctor_t> m;
        m["aes-256-cfb"] = EVP_aes_256_cfb;
        return m;
    }

    void init_evp() {
        static bool initialized = false;
        if (!initialized) {
            initialized = true;
            OpenSSL_add_all_algorithms();
        }
    }

    const static std::map<std::string, cipher_ctor_t> name_cipher_map = create_name_cipher_map();

    const EVP_CIPHER* OpenSSLCrypto::get_cipher_by_name(const std::string& cipher_name){
        init_evp();
        return EVP_get_cipherbyname(cipher_name.c_str());
    }

    OpenSSLCrypto::OpenSSLCrypto(const std::string& cipher_name, 
            unsigned char* key, 
            unsigned char* iv, 
            int op) {
        init_evp();
        std::cout << "INIT OpenSSLCrypto" << std::endl;
        this->cipher_ctx = NULL;
        this->cipher_ctx = EVP_CIPHER_CTX_new();
        const EVP_CIPHER* cipher = get_cipher_by_name(cipher_name);
        if (cipher == NULL) {
            return;
        }
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

