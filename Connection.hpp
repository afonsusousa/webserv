#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <string>
#include <netinet/in.h>
#include "HttpRequest.hpp"
#include "LocalResource.hpp"

class Connection {
	public:
		int                 fd;
		struct sockaddr_in  addr;
		std::string         read_buffer;
		std::string         write_buffer;
		HttpRequest         request;
		LocalResource*      resource;

		Connection(int fd, struct sockaddr_in addr);
		~Connection();

		bool handle_read();
		bool handle_write();
		bool process();
};

#endif
