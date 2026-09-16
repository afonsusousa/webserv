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
#include "Config.hpp"

class Server {
	private:
		int                                                  epoll_fd;
		std::map<int, ListenAddress>                         listening_sockets;
		std::map<int, Connection*>                           clients;
		struct epoll_event                                   events[MAX_EVENTS];
		Config*                                              config;

		void set_nonblocking(int fd);
		void update_epoll(int fd, int events_flags);
		void accept_connections(int listen_fd);
		void process_client(struct epoll_event& event);

	public:
		Server(Config* parsed_config);
		~Server();

		void run();
};

#endif
