#include "CgiResource.hpp"

CgiResource::CgiResource(const std::string& base_path, const std::string& req_uri, const HttpRequest& req) 
	: is_done(false) {
	script_path = base_path + req_uri;
	(void)req;
}

CgiResource::~CgiResource() {}

bool CgiResource::process(std::string& write_buffer) {
	// so much will have to go here
	if (!is_done) {
		std::string body = "CGI Execution Not Implemented Yet\n";
		std::string response = "HTTP/1.1 501 Not Implemented\r\n";
		response += "Content-Type: text/plain\r\n";
		response += "Content-Length: 34\r\n\r\n";
		response += body;

		write_buffer += response;
		is_done = true;
	}
	return true; 
}
