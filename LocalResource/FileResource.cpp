#include "FileResource.hpp"
#include <sstream>
#include <iostream>

#define CHUNK_SIZE 8192

FileResource::FileResource(const std::string& base_path, const std::string& req_uri) : headers_sent(false) {
	path = base_path + req_uri;
	if (path.length() > 0 && path[path.length() - 1] == '/') {
		path += "index.html"; // Basic fallback
	}
	file.open(path.c_str(), std::ios::in | std::ios::binary);
}

FileResource::~FileResource() {
	if (file.is_open()) {
		file.close();
	}
}

void FileResource::generate_headers(std::string& write_buffer) {
	if (!file.is_open()) {
		std::string response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
		write_buffer += response;
		headers_sent = true;
		return;
	}

	// get dirty file size
	file.seekg(0, std::ios::end);
	std::streampos size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::stringstream ss;
	ss << size;

	std::string response = "HTTP/1.1 200 OK\r\n";
	response += "Content-Type: text/plain\r\n"; // Hardcoded
	response += "Content-Length: " + ss.str() + "\r\n\r\n";

	write_buffer += response;
	headers_sent = true;
}

bool FileResource::process(std::string& write_buffer) {
	if (!headers_sent) {
		generate_headers(write_buffer);
		if (!file.is_open()) {
			return true; // Finished (404)
		}
		return false; // send headers and dont block even if its shit
	}

	if (file.is_open() && !file.eof()) {
		char buffer[CHUNK_SIZE];
		file.read(buffer, CHUNK_SIZE);
		std::streamsize bytes_read = file.gcount();
		if (bytes_read > 0) {
			write_buffer.append(buffer, bytes_read);
		}
	}

	return file.eof() || file.fail();
}
