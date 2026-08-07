#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/epoll.h>
#include "Server.hpp"

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
