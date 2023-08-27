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
# include <csignal>


# include <cerrno>
# include <sys/epoll.h>
# include <list>
# include <set>

# include "../Message/Message.hpp"
# include "../Client/Client.hpp"
# include "../Channel/Channel.hpp"
# include "../utills.hpp"

class Message;
class Client;
class Channel;

class Server {
	private:
		int				server_sock;
		int				port;
		int				temp_fd;
		std::string hostname;
		std::string			password;
		int epollfd; // Member variable to hold epoll file descriptor
		std::vector<pollfd>	socket_poll; // Vector to store pollfd structs
	    std::map<int, std::string> client_buffers; // Map to store client message buffers
		std::vector<int> sockets_to_close;
		std::list<Client> clients;
		std::list<Channel> channels;
		std::set<int> addedSockets;  // Store added client sockets
    	int memberValue;
		
	public:
		Server() : memberValue(0) {}
		void setMemberValue(int newValue) {
        	memberValue = newValue;
	    	}
		static Server *serverInstance;
		static void handleSIGINT(int signum) {
			(void)signum;
        	if (serverInstance) {
            	std::cout << "Received SIGINT (Ctrl+C) or SIGQUIT (Ctrl+\\)" << std::endl;
            	serverInstance->setMemberValue(42); // Change the member value to 42.
			}
   		}
	
	
	
	
		typedef std::vector<pollfd>::iterator poll_iterator;
		Server(int port, const char *password);
		~Server();
        int createSocket();
		
		void start();
        void newClient(int);
		void handleDisconnect(int);
		void newMessage(int);
		void authMessage(std::string, Client&);
		void regular_message(std::string, Client&);
		int getNickFD(std::string);
		void notifyAllClientsInChannel(std::string channelName, std::string message, Client &client);
		std::string getClientNick(int clientfd);
		Client getClient(int clientfd);
		int getUserFD(std::string user);
		void handdleSigint(int);


		
		
		//commands
		int   nick(std::string cmd, Client &); //returs 0 if ok, 1 if invalid nick, 2 if nick already in use, 3 if nick is the same as the current one
		void  quit(std::string cmd, Client&);
		void  join(std::string cmd, Client &); 
		void  part(std::string cmd, Client &);
		void  names(std::string cmd, Client &);
		void  list(std::string cmd, Client &);
		void  topic(std::string cmd, Client &);
		void  invite(std::string cmd, Client &);
		void  kick(std::string cmd, Client &);
		void  privmsg(Message message, Client &client);
		void  who(std::string cmd, Client &);
		void  mode(std::string cmd, Client &);

};

#endif
