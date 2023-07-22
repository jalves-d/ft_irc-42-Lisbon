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
