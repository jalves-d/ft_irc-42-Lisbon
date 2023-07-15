#include "Server.hpp"

Server::Server(int *port, const char *password)
{
	this->port = port;
	this->password = password;
	this->server_sock = createSocket();
}

Server::~Server() {}

int Server::createSocket()
{
	//creating an endpoint for communication, socket() return a filedescriptor.
	//AF_INET is used to choose THE IPV4 Internet Protocol.
	//TCP (SOCK_STREAM) is a connection-based protocol. The connection is established and the two parties have a conversation until the connection is terminated by one of the parties or by a network error.
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		std::cout << "Socket creation failed!";
		exit(-1);
	}
	//setting the options to socket use the level specified to TCP comunicate using SOL_SOCKET.
	//SO_REUSEADDR is used to bind() be allowed to reuse local addresses.
	int options = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
	{
		std::cout << "Socket options setting failed!";
		exit(-1);
	}
	//defining I/O operation as non-blocking.
	if (fcntl(sock, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cout <<"Sokcet flag options setting failed!";
		exit(-1);
	}
	//struct used to handle addresses.
	struct sockaddr_in server = {};
	server.sin_family = AF_INET;
    server.sin_port = htons(this->port);
	//INADDR_ANY running machine address.
    server.sin_addr.s_addr = INADDR_ANY;
    memset(&server.sin_zero, 0, 8);
	//Assigning the address specified by addr to the socket referred to by the file descriptor. Specifing the size in bytes of the address structure pointed to by addr.
	//Traditionally, this operation is called "assigning a name to a socket".
	if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        cout << "Binding socket operation failed!" << endl;
        exit(-1);
    }
	//Marking socket in connection-mode, specifying the socket as argument, as accepting connections.
	if (listen(sock, 100) < 0)
    {
        cout << "Listening on socket failed!." << endl;
        exit(-1);
    }
	return sock;
}
