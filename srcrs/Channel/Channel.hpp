#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include "../Client/Client.hpp"

class Client;

class Channel {
    public:
        std::string channelName;
		std::pair<Client *, int> admin;
        std::map<Client *, int> channelUsers;
		std::vector<std::string> kickedUsers;
		std::string topic;
        std::string password;
		typedef std::map<Client *, int>::iterator channellUsersIt;

		Channel(Channel const&);
		Channel &operator=(Channel const &channel);
		Channel();
		Channel(std::string, Client&);
		~Channel();
		bool verifyAdminPrivilege(std::string clientNick);
		void setChannel(std::string topic) { this->topic = topic;}
};

#endif
