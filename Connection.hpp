#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <string>
#include <map>
#include <netinet/in.h>
#include "HttpRequest.hpp"
#include "LocalResource.hpp"
#include "Config.hpp"

class Connection {
	public:
		int                                      fd;
		struct sockaddr_in                       addr;
		std::string                              read_buffer;
		std::string                              write_buffer;
		HttpRequest                              request;
		LocalResource*                           resource;
		std::map<std::string, ServerBlock*>      vhosts;

		Connection(int fd, struct sockaddr_in addr, std::map<std::string, ServerBlock*>& vhosts);
		~Connection();

		bool handle_read();
		bool handle_write();
		bool process();
};

#endif
