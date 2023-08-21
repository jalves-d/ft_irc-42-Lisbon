#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include "../Client/Client.hpp"
# include "../Server/Server.hpp"


class Client;
class Server;

class Channel
{
    public:
        std::string channelName;
        std::map<Client *, int> channelUsers;
		std::string topic;
        std::string password;
		bool adminOnlyTopic;
		typedef std::map<Client *, int>::iterator channellUsersIt;
		int usersLimit;
		bool inviteOnly;

		Channel(Channel const&);
		Channel &operator=(Channel const &channel);
		Channel();
		Channel(std::string, Client&);
		~Channel();
		bool verifyAdminPrivilege(std::string clientNick);
		bool verifyUserInChannel(std::string clientNick);
		bool changeAdminPrivilege(std::string clientNick, bool ad);
		bool setChannel(std::string topic, Server &server);
		bool setUsersLimit(int limit);
		void setPassword( std::string pass ){ this->password = pass;};
		void changeAOT(bool ad){ this->adminOnlyTopic = ad};
		void changeInviteOnly(bool ad){ this->inviteOnly = ad};
};

#endif
