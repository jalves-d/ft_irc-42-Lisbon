#include "Server.hpp"
#include <iostream>
#include <csignal>

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cout << "./ircserv <port> <password>" << std::endl;
		exit(-1);
	}
	int port = std::atoi(argv[1]);
	if (port < 1)
	{
		std::cout << "Invalid port number!" << std::endl;
		exit(-1);
	}
	Server server(std::atoi(argv[1]), argv[2]);
	server.start(server);
}
