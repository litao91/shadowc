#include "cryptor.hpp"
#include "crypto/openssl_crypto.hpp"
#include <openssl/evp.h>
#include <string.h>
#include <iostream>
#include <stdlib.h>

namespace  cryptor{
const int CIPHER_ENC_ENCRYPTION = 1;
const int CIPHER_ENC_DECRYPTION = 0;

static char* random_string(int length) {
    char* r = new char[length];
    FILE* fp = fopen("/dev/urandom", "r");
    fread(r, 1, length, fp);
    fclose(fp);
    return r;
}

Cryptor::Cryptor(const char* password, const char* method) {
    this->iv_sent = false;
    unsigned char* iv =(unsigned char*) random_string( EVP_MAX_IV_LENGTH); 
    this->cipher = this->get_cipher(password, method, CIPHER_ENC_ENCRYPTION, 
            iv);
    delete [] iv;
    this-> cipher_iv = NULL;
    this-> decipher_iv = NULL;
    this->cipher_iv_len = 0;
    this->decipher = NULL;
    this->method = method;
    this->password = password;
}

Cryptor::~Cryptor() {
    if (this->cipher != NULL) {
        delete this->cipher;
    }
    if (cipher_iv != NULL) {
        delete [] cipher_iv;
    }
    if (this->decipher != NULL) {
        delete this->decipher;
    }
    if (this->decipher_iv != NULL) {
        delete this->decipher_iv;
    }
}

void Cryptor::set_cipher_iv(unsigned char* iv) {
    if (this->cipher_iv != NULL) {
        delete [] this->cipher_iv;
        this->cipher_iv = NULL;
    }
    this->cipher_iv = iv;
}

crypto::Crypto* Cryptor::get_cipher(
        const char* password,
        const char* method,
        int op, 
        unsigned char* iv) {
    // We hard code the keylen for aes-256-cfb for now
    unsigned char key[EVP_MAX_KEY_LENGTH], iv_buf[EVP_MAX_IV_LENGTH];

    const unsigned char* salt = NULL;
    const EVP_CIPHER* cipher = crypto::OpenSSLCrypto::get_cipher_by_name(method);
    const EVP_MD *dgst = EVP_get_digestbyname("md5");
    if (!dgst) {
        std::cerr << "Error: no such digest" << std::endl;
        return NULL;
    }
    if (!EVP_BytesToKey(cipher, dgst, salt,
                (const unsigned char*) password,
                strlen(password), 1, key, iv_buf)) {
        std::cerr << "EVP_BytesToKey failed" << std::endl;
        return NULL;
    }
    crypto::OpenSSLCrypto* crypto =  new crypto::OpenSSLCrypto(method, key, iv, op);
    if (op == CIPHER_ENC_ENCRYPTION) {
        int iv_len = cipher->iv_len;
        unsigned char* buf = new unsigned char[iv_len];
        memcpy(buf, iv, sizeof(unsigned char) * iv_len);
        this->set_cipher_iv(buf);
        this->cipher_iv_len = iv_len;
    }
    return crypto;
}

void Cryptor::encrypt(const unsigned char* buf, int len,
        unsigned char* out, int* out_size) {
    if (len == 0) {
        *out_size = 0;
        return;
    }
    if(this->iv_sent) {
        return this->cipher->update(
                buf, len, out, out_size);
    } else {
        memcpy(out, this->cipher_iv, this->cipher_iv_len * sizeof(unsigned char));
        int encrypted_len = 0;
        this->cipher->update(
                buf, len, out + this->cipher_iv_len, &encrypted_len);
        *out_size = encrypted_len + this->cipher_iv_len;
    }
}

void Cryptor::decrypt(const unsigned char* buf, int len, 
        unsigned char* out, int* out_size) {
    if (len == 0) {
        *out_size = 0;
        return;
    }
    if (this->decipher == NULL) {
        unsigned char* decipher_iv = new unsigned char[this->cipher_iv_len];
        memcpy(decipher_iv, buf, this->cipher_iv_len * sizeof(unsigned char));
        this->decipher_iv = decipher_iv;
        this->decipher = this->get_cipher(
                this->password,
                this->method,
                CIPHER_ENC_DECRYPTION,
                decipher_iv);
        buf = buf + this->cipher_iv_len;
        len = len - this->cipher_iv_len;
    }
    if (len == 0) {
        *out_size = 0;
        return;
    }
    this->decipher->update(
            buf, len, out, out_size);
    return;
}
}
