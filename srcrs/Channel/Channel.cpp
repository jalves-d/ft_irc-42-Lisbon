#include "Channel.hpp"

Channel::Channel()
{
	this->channelName = "defaultChannel";
	this->password = "";
}

Channel::Channel(std::string name, Client &admin)
{
    this->admin = std::make_pair(&admin, 1);
    this->channelName = name;
    channelUsers.insert(std::make_pair(&admin, 1));
}

Channel::Channel(Channel const &channel)
{
    *this = channel;
}

Channel &Channel::operator=(Channel const &channel)
{
    if (this == &channel)
        return(*this);
    return (*this);
}

Channel::~Channel() {}

bool Channel::verifyAdminPrivilege(std::string clientNick)
{
	std::map<Client*, int> users;
    Channel::channellUsersIt it;
    std::string message;

    users = this->admin;
    for (it = users.begin(); it != users.end(); it++)
	{
        if ((it)->first->nickname != clientNick)
			return true;
    }
	return false;
}
