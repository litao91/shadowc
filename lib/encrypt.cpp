#ifndef SHADOW_C_ENCRYPT_H__
#define SHADOW_C_ENCRYPT_H__

namespace encrypt {
    enum Method {
        AES_256_CFB = 0
    };

    class Encryptor {
        public:
            Encryptor(const char* password, Method method);
            char* decrypt(const char* buf);
            char* encrypt(const char* buf);

        private:
            const char* password;
    };
}
#endif
