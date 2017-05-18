cmake_minimum_required (VERSION 3.1)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
project (shadowc)
include_directories ("${PROJECT_SOURCE_DIR}/include")

find_library(SSL_LIB libssl.a REQUIRED)
find_library(CRYPTO_LIB libcrypto.a REQUIRED)
find_library(DL libdl.so REQUIRED)

add_library(eventloop STATIC "${PROJECT_SOURCE_DIR}/lib/eventloop.cpp")
target_compile_options(eventloop PUBLIC -std=c++11 -Wall)

add_library(utils STATIC "${PROJECT_SOURCE_DIR}/lib/utils.cpp")
target_compile_options(utils PUBLIC -std=c++11 -Wall)

add_library(asyncdns STATIC "${PROJECT_SOURCE_DIR}/lib/asyncdns.cpp")
target_link_libraries(asyncdns eventloop utils)
target_compile_options(asyncdns PUBLIC -std=c++11 -Wall)


add_library(openssl_crypto STATIC "${PROJECT_SOURCE_DIR}/lib/crypto/openssl_crypto.cpp")
target_link_libraries(openssl_crypto ${CRYPTO_LIB} ${SSL_LIB} ${DL} )
target_compile_options(openssl_crypto PUBLIC -std=c++11 -Wall)

add_library(cryptor STATIC "${PROJECT_SOURCE_DIR}/lib/cryptor.cpp")
target_link_libraries(cryptor openssl_crypto utils)
target_compile_options(cryptor PUBLIC -std=c++11 -Wall)

add_library(tcprelay STATIC "${PROJECT_SOURCE_DIR}/lib/tcprelay.cpp")
target_link_libraries(tcprelay eventloop utils cryptor asyncdns)
target_compile_options(tcprelay PUBLIC -std=c++11 -Wall)

add_executable(server "${PROJECT_SOURCE_DIR}/src/server.cpp")
target_link_libraries(server asyncdns)
target_compile_options(server PUBLIC -std=c++11 -Wall)

add_executable(test_asyncdns "${PROJECT_SOURCE_DIR}/test/test_asyncdns.cpp")
target_link_libraries(test_asyncdns asyncdns)
target_compile_options(test_asyncdns PUBLIC -std=c++11 -Wall)

add_executable(test_crypto "${PROJECT_SOURCE_DIR}/test/test_crypto.cpp")
target_link_libraries(test_crypto openssl_crypto)
target_compile_options(test_crypto PUBLIC -std=c++11 -Wall)

add_executable(test_cryptor "${PROJECT_SOURCE_DIR}/test/test_cryptor.cpp")
target_link_libraries(test_cryptor cryptor utils)
target_compile_options(test_cryptor PUBLIC -std=c++11 -Wall)

