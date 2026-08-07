#include "Connection.hpp"
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>

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
    if (!request.parse(read_buffer)) {
        return true; // wait for more data
    }

    if (request.state == HttpRequest::COMPLETE) {
        std::string response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: text/plain\r\n";

        std::string body_content = "Hello from Webserv!\n";
        body_content += "Method: " + request.method + "\n";
        body_content += "URI: " + request.uri + "\n";
        body_content += "Body: " + request.body + "\n";

        std::stringstream ss;
        ss << body_content.size();
        response += "Content-Length: " + ss.str() + "\r\n\r\n";
        response += body_content;

        write_buffer += response;
        request.reset();
    } else if (request.state == HttpRequest::ERROR) {
        std::string response = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
        write_buffer += response;
        request.reset();
    }

    return true;
}
