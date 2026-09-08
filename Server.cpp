#include "Server.hpp"
#include <stdexcept>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <errno.h>

void Server::set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return;
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void Server::update_epoll(int fd, int events_flags) {
    struct epoll_event ev;
    ev.events = events_flags;
    ev.data.fd = fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev);
}

Server::Server(const char* ip, int port) {
    socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_fd == -1) {
        throw std::runtime_error("socket failed");
    }

    int opt = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    set_nonblocking(socket_fd);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);

    if (bind(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        throw std::runtime_error("bind failed");
    }

    if (listen(socket_fd, SOMAXCONN) == -1) {
        throw std::runtime_error("listen failed");
    }

    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        throw std::runtime_error("epoll_create1 failed");
    }

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = socket_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event) == -1) {
        throw std::runtime_error("epoll_ctl failed");
    }
}

Server::~Server() {
    for (std::map<int, Connection*>::iterator it = clients.begin(); it != clients.end(); ++it) {
        delete it->second;
    }
    close(socket_fd);
    close(epoll_fd);
}

void Server::accept_connections() {
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size = sizeof(clnt_addr);

    while (true) {
        int c_sock = accept(socket_fd, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
        if (c_sock == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break; // Processed all incoming connections
            }
            break;
        }

        set_nonblocking(c_sock);

        struct epoll_event ev;
        ev.events = EPOLLIN;
        ev.data.fd = c_sock;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, c_sock, &ev) == -1) {
            close(c_sock);
        } else {
            clients[c_sock] = new Connection(c_sock, clnt_addr);
        }
    }
}

void Server::process_client(struct epoll_event& event) {
	int c_sock = event.data.fd;
	Connection* conn = clients[c_sock];
	bool ok = true;

	if (event.events & EPOLLIN) {
		ok = conn->handle_read();
	}

	if (ok && (event.events & EPOLLOUT)) {
		ok = conn->handle_write();
	}

	if (ok) {
		conn->process();
	}

	if (!ok) {
		epoll_ctl(epoll_fd, EPOLL_CTL_DEL, c_sock, NULL);
		delete conn;
		clients.erase(c_sock);
	} else {
		int event_flags = EPOLLIN;
		if (!conn->write_buffer.empty() || conn->resource) {
			event_flags |= EPOLLOUT;
		}
		update_epoll(c_sock, event_flags);
	}
}

void Server::run() {
    while (1) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            throw std::runtime_error("epoll_wait failed");
        }

        for (int n = 0; n < nfds; ++n) {
            if (events[n].data.fd == socket_fd) {
                accept_connections();
            } else {
                process_client(events[n]);
            }
        }
    }
}
