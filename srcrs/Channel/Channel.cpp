#include "Channel.hpp"

Channel::Channel()
{
	this->channelName = "defaultChannel";
	this->password = "";
    this->usersLimit = 10;
}

Channel::Channel(std::string name, Client &admin)
{
    this->channelName = name;
    channelUsers.insert(std::make_pair(&admin, 1));
    this->usersLimit = 10;
    this->password = "";
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

void Channel::removeUserFromKickedList(std::string clientNick)
{
    for (std::vector<std::string>::iterator it = this->kickedUsers.begin(); it != this->kickedUsers.begin(); ++it)
    {
        if (*it == clientNick)
			this->kickedUsers.erase(it);  
    }
}

bool Channel::verifyUserInChannel(std::string clientNick)
{
    for (channellUsersIt it = this->channelUsers.begin(); it != this->channelUsers.end(); it++)
	{
        if ((it)->first->nickname == clientNick)
			return true;
    }
	return false;
}

bool Channel::changeAdminPrivilege(std::string clientNick)
{
	for (channellUsersIt it = this->channelUsers.begin(); it != this->channelUsers.end(); it++)
	{
        if ((it)->first->nickname == clientNick && (it)->second == 1)
		{
			(it)->second = 0;
			return true;
		}
		else if((it)->first->nickname == clientNick && (it)->second == 0)
		{
			(it)->second = 1;
			return true;
		}
	}
	return false;
}

bool Channel::setChannel(std::string topic, Server &server)
{
    Channel *nullChannel;

    *nullChannel = server.getChannel(topic);
    if (nullChannel == NULL)
    {
        this->topic = topic;
        return true;
    }
    return false;
}

bool Channel::setUsersLimit(int limit)
{
    if (limit > 0 && limit >= this->channelUsers.size())
    {
        this->usersLimit = limit;
        return true;
    }
    return false;
}
