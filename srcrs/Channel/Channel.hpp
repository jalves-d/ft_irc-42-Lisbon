#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include "../Client/Client.hpp"
#include "../Server/Server.hpp"

class Client;

class Channel
{
    public:
        std::string channelName;
        std::map<Client *, int> channelUsers;
		std::vector<std::string> kickedUsers;
		std::string topic;
        std::string password;
		typedef std::map<Client *, int>::iterator channellUsersIt;
		int usersLimit;

		Channel(Channel const&);
		Channel &operator=(Channel const &channel);
		Channel();
		Channel(std::string, Client&);
		~Channel();
		bool verifyAdminPrivilege(std::string clientNick);
		bool setChannel(std::string topic, Server &server);
		bool setUsersLimit(int limit);
		bool removeUsersLimit(){ this->usersLimit = -1; };
		void setPassword( std::string pass ){ this->password = pass;}
};

#endif
