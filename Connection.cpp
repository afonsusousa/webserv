#include "Connection.hpp"
#include <algorithm>
#include <cctype>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <cstdio>
#include <cstdlib>

#define buffersize 1024

Connection::Connection(int fd, struct sockaddr_in addr) : fd(fd), addr(addr) {}

Connection::~Connection() {
    close(fd);
}

bool Connection::handle_read() {
    char buffer[buffersize];
    while (true) {
        int bytes_read = recv(fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            read_buffer += buffer;
        } else if (bytes_read == 0) {
            return false; // client disconnected
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break; // no more data
            }
            return false; // error
        }
    }
    return true;
}

bool Connection::handle_write() {
    if (write_buffer.empty()) {
        return true;
    }

    int bytes_sent = send(fd, write_buffer.c_str(), write_buffer.size(), 0);
    if (bytes_sent > 0) {
        write_buffer.erase(0, bytes_sent);
        return true;
    } else if (bytes_sent == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return true; // Wait for next EPOLLOUT
        }
        return false; // Error
    }
    return true;
}

bool Connection::process() {
    size_t pos = read_buffer.find('\n');
    if (pos != std::string::npos) {
        std::string line = read_buffer.substr(0, pos);
        read_buffer.erase(0, pos + 1);

        for (std::string::iterator it = line.begin(); it != line.end(); ++it) {
            *it = std::toupper(static_cast<unsigned char>(*it));
        }
        write_buffer += line + "\n";
    }
    return true;
}
