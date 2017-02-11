#include "crypto/openssl_crypto.hpp"
#include "crypto/crypto.hpp"
#include <iostream>
#include <time.h>
#include <stdlib.h>

void run_cipher(crypto::Crypto* cipher, crypto::Crypto* decipher) {
    const int BLOCK_SIZE = 16384;
    int rounds = 1024;
    FILE* fp = fopen("/dev/urandom", "r");
    unsigned char* plain = new unsigned char[BLOCK_SIZE * rounds];
    fread(plain, 1, BLOCK_SIZE * rounds, fp);
    fclose(fp);
    int pos = 0; 
    srand(time(NULL));

    std::cout << "Test start" << std::endl;
    unsigned char* encrypted = new unsigned char[16384 * 1024];
    int result_pos = 0;
    
    while(pos < BLOCK_SIZE * rounds - 1) {
        std::cout << " POS:" << pos << std::endl;
        std::cout << "RPOS:" << result_pos << std::endl;
        int l = rand() % 32688 + 100;
        l = pos + l < 16384 * 1024 ? l : 16384 * 1024 - pos - 1;
        int result_size = 0;
        cipher->update(plain + pos, 
                l, encrypted + result_pos, 
                &result_size);
        result_pos += result_size;
        pos += l;
    }

    unsigned char* decrypted = new unsigned char[16384 * 1024];
    pos = 0;
    result_pos = 0;
    while(pos < BLOCK_SIZE * rounds - 1) {
        int l = rand() % 32668 + 100;
        l = pos + l < 16384 * 1024 ? l : 16384 * 1024 - pos - 1;
        int result_size = 0;
        decipher->update(encrypted+pos,
                l, decrypted + result_pos,
                &result_size);
        std::cout << "From " << pos << " to " << pos + l << std::endl;
        result_pos += result_size;
        pos += l;
    }

    for (int i = 0; i < BLOCK_SIZE * rounds; ++i) {
        if (plain[i] != decrypted[i]) {
            std::cerr << plain[i] << " != " << decrypted[i] << std::endl;
        }
    }
    std::cout << "All good!" << std::endl;
    delete [] plain;
    delete [] encrypted;
    delete [] decrypted;
}

int main(int argc, char** argv) {
    const char* name = "aes-256-cfb";
    unsigned char key[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    unsigned char iv[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    crypto::OpenSSLCrypto cipher = crypto::OpenSSLCrypto(name, 
            key, iv, 1);
    crypto::OpenSSLCrypto decipher = crypto::OpenSSLCrypto(name, 
            key, iv, 0);
    run_cipher(&cipher, &decipher);
}

