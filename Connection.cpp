#include "Connection.hpp"
#include "FileResource.hpp"
#include "CgiResource.hpp"
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>

#define buffersize 1024

Connection::Connection(int fd, struct sockaddr_in addr) : fd(fd), addr(addr), resource(NULL) {}

Connection::~Connection() {
	if (resource) {
		delete resource;
	}
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
		if (!resource) {
			std::string base_path = "."; //will come from config later

			if (request.uri.length() >= 4 && request.uri.substr(request.uri.length() - 4) == ".cgi") {
				//everything CGI goes here
				resource = new CgiResource(base_path, request.uri, request);
			} else {
				resource = new FileResource(base_path, request.uri);
			}
		}

		if (write_buffer.size() < 65536) { //64KB max
			if (resource->process(write_buffer)) {
				request.reset();
				delete resource;
				resource = NULL;
			}
		}

	} else if (request.state == HttpRequest::ERROR) {
		std::string response = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
		write_buffer += response;
		request.reset();
		if (resource) {
			delete resource;
			resource = NULL;
		}
	}

	return true;
}
