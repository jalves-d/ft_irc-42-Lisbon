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
#include "../Cmd/Cmd.hpp"
#include "../Channel/Channel.hpp"

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
		Server(int port, const char *password);
		~Server();
		Client *getClient() {return clients[temp_fd];}
		std::map<int, Client *>& getClients() { return clients; }
		std::vector<Channel*> &getChannels() {return channels;}
		std::vector<pollfd> &getPoll() { return socket_poll;}
		Channel &getChannel(std::string &);
		int createSocket();
		void start();
		void connectNewClient();
		void disconnectClient(int sock, Client *client, std::string msg);
		void newMessage(int);
		void notifyAllClients(Channel const *channel, Client &client, std::string msg);
		std::string getPassword() { return password;}
		int verifyUserInChannel(std::string move, Channel &channel);
		void Server::invite(std::string cmd, Client &client);
		void Server::kick(std::string cmd, Client &client);
		void Server::topic(std::string cmd, Client &client);
};

#endif
