#ifndef SERVER_HPP
# define SERVER_HPP

#include <iostream>
#include <unistd.h>
#include <cstdio>
#include <string.h>
#include <vector>
#include <map>
#include <csignal>
#include <sstream>
#include <functional>
#include <cctype>
#include <locale>
#include <netdb.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <algorithm>
#include <poll.h>

#include "../Client/Client.hpp"

class Server {
	private:
		int				server_sock;
		int				port;
		int				temp_fd;
		std::string			password;
		std::vector<pollfd>	socket_poll;
		std::vector<Channel*> channels;
		std::map<int, Client *> clients;
	public:
		typedef std::vector<Channel*>::iterator channel_iterator;
		typedef std::vector<pollfd>::iterator poll_iterator;
		typedef std::map<int, Client*>::iterator client_iterator;
		Server(const char *port, const char *password);
		~Server();
		Client *getClient() {return clients[temp_fd];}
		std::map<int, Client *>& getClients() { return clients; }
		std::vector<Channel*> &getChannel() {return channels;}
		std::vector<pollfd> &getPoll() { return socket_poll;}
		Channel &getChannel(std::string &);
		int createSocket();
		void startServer(Server &);
		void connectNewClient();
		void disconnectClient(int sock)
		void newMessage(int);
		void notifyAllClients(Channel const *, Client &);
		std::string getPassword() { return password;}
};

#endif
