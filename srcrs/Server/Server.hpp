#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <unistd.h>
# include <cstdio>
# include <string.h>
# include <vector>
# include <map>
# include <csignal>
# include <sstream>
# include <functional>
# include <cctype>
# include <locale>
# include <netdb.h>
# include <sys/poll.h>
# include <sys/socket.h>
# include <fcntl.h>
# include <algorithm>
# include <poll.h>
# include <cerrno>


# include "../Message/Message.hpp"
# include "../Client/Client.hpp"

class Message;
class Client;

class Server {
	private:
		int				server_sock;
		int				port;
		int				temp_fd;
		std::string			password;
		std::vector<pollfd>	socket_poll; // Vector to store pollfd structs
	    std::map<int, std::string> client_buffers; // Map to store client message buffers
		std::vector<int> sockets_to_close;
		std::vector<Client> clients;

	public:
		typedef std::vector<pollfd>::iterator poll_iterator;
		Server(int port, const char *password);
		~Server();
        int createSocket();
		
		void start();
        void connectNewClient();
		void disconnectClient(int);
		void newMessage(int);
		Message authMessage(int);
		Client autenthicateNewClient(int);

		
};

#endif
