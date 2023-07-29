#include "Channel.hpp"

Channel::Channel()
{
	this->channelName = "defaultChannel";
	this->password = "";
}

Channel::Channel(std::string name, Client &admin)
{
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
    for (channellUsersIt it = this->channelUsers.begin(); it != this->channelUsers.end(); it++)
	{
        if ((it)->first->nickname == clientNick && (it)->second == 1)
			return true;
    }
	return false;
}
