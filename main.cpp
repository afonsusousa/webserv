#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#include "Server.hpp"
#include "Parser/ConfigParser.hpp"
#include "Config.hpp"

int main(int argc, char* argv[]) {
	std::string config_path = "default.conf";
	if (argc > 1) {
		config_path = argv[1];
	}

	Config* config = NULL;
	try {
		ConfigParser parser(config_path);
		config = parser.parse();
	} catch (const std::exception& e) {
		std::cerr << "Config Error: " << e.what() << "\n";
		return 1;
	}

	if (!config) {
		std::cerr << "Config Error: Failed to generate configuration AST.\n";
		return 1;
	}
	
	try {
		Server server(config);
		std::cout << "Server starting successfully..." << std::endl;
		server.run();
	} catch (const std::exception& e) {
		std::cerr << "Server Exception: " << e.what() << "\n";
		delete config;
		return 1;
	}
	
	return 0;
}
