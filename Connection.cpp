#include "Connection.hpp"
#include "FileResource.hpp"
#include "CgiResource.hpp"
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <sstream>

#define buffersize 1024

Connection::Connection(int fd, struct sockaddr_in addr, std::map<std::string, ServerBlock*>& mapped_vhosts) 
	: fd(fd), addr(addr), resource(NULL), vhosts(mapped_vhosts) {}

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

ServerBlock* Connection::get_server_block() {
	std::string host = "";
	if (request.headers.count("Host")) {
		host = request.headers["Host"];
		size_t colon = host.find(':');
		if (colon != std::string::npos) {
			host = host.substr(0, colon);
		}
	}

	if (vhosts.count(host)) {
		return vhosts[host];
	} else if (vhosts.count("")) {
		return vhosts[""]; // Explicit default
	} else if (!vhosts.empty()) {
		return vhosts.begin()->second; // Implicit fallback
	}
	return NULL;
}

LocationConfig* Connection::get_location(ServerBlock* server) {
	if (!server) return NULL;

	std::string match_path = "";
	LocationConfig* best_match = NULL;
	
	std::map<std::string, LocationConfig>::iterator it;
	for (it = server->locations.begin(); it != server->locations.end(); ++it) {
		if (request.uri.find(it->first) == 0) {
			if (it->first.length() > match_path.length()) {
				match_path = it->first;
				best_match = &it->second;
			}
		}
	}
	return best_match;
}

bool Connection::handle_redirect(LocationConfig* loc) {
	if (!loc || loc->redirect.first == 0) return false;

	int redirect_code = loc->redirect.first;
	std::string redirect_url = loc->redirect.second;
	std::string status_text = "Found"; 

	if (redirect_code == 301) status_text = "Moved Permanently";
	else if (redirect_code == 302) status_text = "Found";
	else if (redirect_code == 303) status_text = "See Other";
	else if (redirect_code == 307) status_text = "Temporary Redirect";
	else if (redirect_code == 308) status_text = "Permanent Redirect";

	std::stringstream ss;
	ss << "HTTP/1.1 " << redirect_code << " " << status_text << "\r\n";
	ss << "Location: " << redirect_url << "\r\n";
	ss << "Content-Length: 0\r\n\r\n";
	
	write_buffer += ss.str();
	request.reset();
	return true;
}

void Connection::initialize_resource() {
	ServerBlock* server = get_server_block();
	LocationConfig* loc = get_location(server);

	if (handle_redirect(loc)) {
		return;
	}

	std::string base_path = ".";
	if (loc && !loc->alias.empty()) {
		base_path = loc->alias;
	}

	if (request.uri.length() >= 4 && request.uri.substr(request.uri.length() - 4) == ".cgi") {
		resource = new CgiResource(base_path, request.uri, request);
	} else {
		resource = new FileResource(base_path, request.uri);
	}
}

bool Connection::process() {
	if (!request.parse(read_buffer)) {
		return true; // wait for more data
	}

	if (request.state == HttpRequest::COMPLETE) {
		if (!resource) {
			initialize_resource();
		}

		if (resource && write_buffer.size() < 65536) { //64KB max
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
