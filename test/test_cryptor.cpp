#include "cryptor.hpp"
#include "utils.hpp"
#include <iostream>

void test_encryptor() {
    unsigned char plain[10240];
    unsigned char plain2[10240];
    unsigned char encrypted[15240];
    int outsize;
    int outsize2;
    const char* method = "aes-256-cfb";
    utils::random_string(10240, (char *)plain);
    cryptor::Cryptor encryptor("key", method);
    cryptor::Cryptor decryptor("key", method);

    encryptor.encrypt(plain, 10240, encrypted, &outsize);
    decryptor.decrypt(encrypted, outsize, plain2, &outsize2);
    bool ok = true;
    for(int i = 0; i < 10240; ++i) {
        if (plain[i] != plain2[i]) {
            std::cout << "error at " << i << std::endl;
            ok = false;
            break;
        }
    }
    if (ok) {
        std::cout << "OK!" << std::endl;
    } else {
        std::cout << "Error!" << std::endl;
    }
}


int main() {
    test_encryptor();
}
