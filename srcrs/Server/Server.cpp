#include "Server.hpp"


Server::Server(int port, const char *password)
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
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &options, sizeof(options)))
	{
		std::cout << "Socket options setting failed!";
		exit(-1);
	}
	//defining I/O operation as non-blocking, in this project is just allowed to use fcntl() using this flags as indicated on the subject.
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
        std::cout << "Binding socket operation failed!" << std::endl;
        exit(-1);
    }
	//Marking socket in connection-mode, specifying the socket as argument, as accepting connections.
	if (listen(sock, 100) < 0)
    {
        std::cout << "Listening on socket failed!." << std::endl;
        exit(-1);
    }
	return sock;
}

void Server::start()
{
    pollfd server_fd = {this->server_sock, POLLIN, 0};
    this->socket_poll.push_back(server_fd);
    std::cout << "Server listening on port: " << this->port << std::endl;

    std::vector<int> sockets_to_close;

    while (true) {
        if (poll(this->socket_poll.data(), this->socket_poll.size(), -1) < 0) {
            std::cout << "An error happened while polling!" << std::endl;
        }

        for (std::vector<pollfd>::iterator it = this->socket_poll.begin(); it != this->socket_poll.end(); ) {
            if (it->revents == 0) {
                ++it;
                continue;
            }

            //std::cout << "loop it->fd = " << it->fd << std::endl;

            if ((it->revents & POLLHUP) == POLLHUP) {
                std::cout << "Client disconnected (sock_fd = " << it->fd << ")" << std::endl;
                close(it->fd); // Close the socket associated with the disconnected client
				disconnectClient(it->fd);
                it = this->socket_poll.erase(it); // Remove the pollfd from the vector
                std::cout << "socketpoll size after delete = " << this->socket_poll.size() << std::endl;
                continue; // Skip the rest of the loop iteration
            }

            if ((it->revents & POLLIN) == POLLIN) {
                if (it->fd == this->server_sock) {
                    connectNewClient();
                    break; // Don't continue to check events for this iteration
                } else {
                    std::cout << "new message for = " << it->fd << std::endl;
                    newMessage(it->fd);
					break;
                }
            }

            ++it; // Increment the iterator here
        }

        // Process and close marked sockets
        for (std::vector<int>::iterator close_it = sockets_to_close.begin(); close_it != sockets_to_close.end(); ++close_it) {
            std::cout << "Client disconnected (sock_fd = " << *close_it << ")" << std::endl;
            close(*close_it); // Close the socket associated with the disconnected client

            for (std::vector<pollfd>::iterator poll_it = this->socket_poll.begin(); poll_it != this->socket_poll.end(); ) {
                if (poll_it->fd == *close_it) {
                    poll_it = this->socket_poll.erase(poll_it);
                } else {
                    ++poll_it;
                }
            }
        }

        sockets_to_close.clear(); // Clear the list of sockets to be closed
    }
}






void Server::connectNewClient()
{
    sockaddr_in temp;
    socklen_t len;
    int clientId;
	std::vector<std::string> client_info;

    len = sizeof(temp);
    clientId = accept(server_sock, (sockaddr *)&temp, &len);
    if (clientId < 0) {
        std::cerr << "An error occurred while accepting new client !" << std::endl;
        return;
    }

    pollfd new_fd = {clientId, POLLIN, 0};
    socket_poll.push_back(new_fd);

	Client new_client = autenthicateNewClient(clientId);
	clients.push_back(new_client);
    std::string str = ":local 001 local Welcome to ft_irc server!\tPlease enter the password: \r\n";
    send(new_client.get_fd(), str.c_str(), str.length(), 0);
}

Client Server::autenthicateNewClient(int client_fd){
	
	Client new_client(client_fd);

	Message msg;
	int i = 0;
	while(i <= 4)
	{
		std::cout << i << std::endl;
		msg = authMessage(client_fd);
		if (i == 0){
			if (msg.get_command().compare("CAP")){
				msg = authMessage(client_fd);
			}
			else if (msg.get_command().compare("PASS")){
				;//check password
			}
			else{
				;//invalid password
			}
		}
		else if (i == 1){
			if (msg.get_command().compare("NICK")){
				//check nickname
				new_client.nickname = msg.get_params();
			}
			else{
				;
			}//must be nick
		}
		else if (i == 2){
			if (msg.get_command().compare("USER")){
				//check username
				new_client.username = msg.get_params();
			}
			else{
				;//must be user
			}
			
		}
		msg = authMessage(client_fd);
		i++;
	}
	return new_client;
}

Message Server::authMessage(int client_fd){
	char buffer[1000];
	int bytes = recv(client_fd, buffer, sizeof(buffer), 0);
	Message msg;
	if (bytes <= 0) {
		if (bytes < 0) {
			std::cerr << "Error while receiving data from client (sock_fd = " << client_fd << ")" << std::endl;
		}
		std::cout << "Client disconnected in message (sock_fd = " << client_fd << ")" << std::endl;
		disconnectClient(client_fd);
		close(client_fd); // Close the socket associated with the disconnected client
		return msg;
	}

	std::string message(buffer, bytes);
	std::cout << "Received authentication message authentication from " << client_fd << ": " << message << std::endl;
	msg.Message_picker(message);
	return msg;
}
    


void Server::newMessage(int sock_fd)
{
    std::cout << "newMessage sock_fd = " << sock_fd << std::endl;

    char buffer[1000];
    int bytes = recv(sock_fd, buffer, sizeof(buffer), 0);

    if (bytes <= 0) {
        if (bytes < 0) {
            std::cerr << "Error while receiving data from client (sock_fd = " << sock_fd << ")" << std::endl;
        }
        std::cout << "Client disconnected in message (sock_fd = " << sock_fd << ")" << std::endl;
		disconnectClient(sock_fd);
        close(sock_fd); // Close the socket associated with the disconnected client
        return;
    }

    std::string message(buffer, bytes);
    std::cout << "Received message from " << sock_fd << ": " << message << std::endl;
	Message msg;
	msg.Message_picker(message);
	if (msg.get_invalid() == true)
	{
		//add error message
		return;
	}
	
    // Handle the complete message here

    // Note: If the message doesn't fit in the buffer, you might need to handle partial messages
}

void Server::disconnectClient(int sock_fd)
{
	std::vector<Client>::iterator it;
	for (it = clients.begin(); it != clients.end(); ++it) {
		if (it->get_fd() == sock_fd) {
			break;
		}
	}
	if (it == clients.end()) {
		std::cout << "disconnectClient: client not found" << std::endl;
		return;
	}
	clients.erase(it);
	std::cout << "disconnectClient sock_fd = " << sock_fd << std::endl;
	this->sockets_to_close.push_back(sock_fd);

	//add disconnect message
}

