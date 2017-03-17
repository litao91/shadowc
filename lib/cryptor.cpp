#include "cryptor.hpp"
#include "crypto/openssl_crypto.hpp"
#include <openssl/evp.h>
#include <string.h>
#include <iostream>
namespace  cryptor{
Cryptor::Cryptor(const char* password, const char* method) {

}

crypto::Crypto* get_cipher(
        const char* password,
        const char* method,
        int op) {
    // We hard code the keylen for aes-256-cfb for now
    unsigned char key[EVP_MAX_KEY_LENGTH], iv[EVP_MAX_IV_LENGTH];

    const unsigned char* salt = NULL;
    const EVP_CIPHER* cipher = crypto::OpenSSLCrypto::get_cipher_by_name(method);
    const EVP_MD *dgst = EVP_get_digestbyname("md5");
    if (!dgst) {
        std::cerr << "Error: no such digest" << std::endl;
        return NULL;
    }
    if (!EVP_BytesToKey(cipher, dgst, salt,
                (const unsigned char*) password,
                strlen(password), 1, key, iv)) {
        std::cerr << "EVP_BytesToKey failed" << std::endl;
        return NULL;
    }
    return new crypto::OpenSSLCrypto(method, key, iv, op);
}
}
