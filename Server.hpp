#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>

#define MAX_EVENTS 64

#include "Connection.hpp"

class Server {
	private:
		int                         socket_fd;
		struct sockaddr_in          server_addr;

		int                         epoll_fd;
		std::map<int, Connection*>  clients;
		struct  epoll_event         events[MAX_EVENTS];

		void set_nonblocking(int fd);
		void update_epoll(int fd, int events_flags);
		void accept_connections();
		void process_client(struct epoll_event& event);

	public:
		Server(const char* ip, int port);
		~Server();

		void run();
};

#endif
