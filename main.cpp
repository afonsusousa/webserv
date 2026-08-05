#include <cstring>
#include <stdexcept>
#include <string>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <vector>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <string>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/epoll.h>
#include "Server.hpp"
std::vector<std::string> getstringsfromfd(int fd) {

    std::vector<std::string> ret;

    char bytes[8];
    int rd = 0;
    std::string str = "";
    std::string bstr = "";

    while (!bstr.empty() || ((rd = read(fd, bytes, sizeof(bytes))) > 0)) {

        if (bstr.empty()) {
            bstr = std::string(bytes, rd);
        }

        auto i = bstr.find("\n", 0, 1);

        if (i == std::string::npos) {
            str += bstr;
            bstr = "";
        } else {
            ret.push_back(str + bstr.substr(0, i));
            str = "";
            bstr = bstr.substr(i + 1);
        }
    }

    if (!str.empty()) {
        ret.push_back(str);
    }


    return ret;
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    try {
        Server server("127.0.0.1", 8080);
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
