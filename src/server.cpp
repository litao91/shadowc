#include <iostream>
#include "cxxopts.hpp"

int main(int argc, char** argv) {
    cxxopts::Options options(argv[0], "Command Options");
    options.add_options()
        ("password", "Password", cxxopts::value<std::string>())
        ("port", "Port", cxxopts::value<int>())
        ("w,workers", "Workers", cxxopts::value<int>());
    options.parse(argc, argv);
    std::string password = options["password"].as<std::string>();
    int port = options["port"].as<int>();
    int num_worker = options["workers"].as<int>();
    std::cout << "============= config ===========" << std::endl;
    std::cout << "Password " << password << std::endl;
    std::cout << "Port: " << port << std::endl;
    std::cout << "Worker: " << num_worker << std::endl;
    std::cout << "================================" << std::endl;
}
