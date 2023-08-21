#include "Client.hpp"

Client::Client() {}

Client::Client(int fd)
{
    this->sock_fd = fd;
    this->avail = 0;
    std::cout << "New client created. FD: " << fd << std::endl;
}

Client::Client(Client const &clt)
{
    this->sock_fd = clt.sock_fd;
    this->nickname = clt.nickname;
    this->username = clt.username;
    this->mod = clt.mod;
    this->avail = clt.avail;
    this->flag = clt.flag;
}

Client &Client::operator=(Client const &clt)
{
    if (this == &clt)
        return(*this);
    return (*this);
}

int Client::available() { return avail; }

std::string Client::getPrefix() const
{
	return this->nickname + ("!" + this->username) + ("@localhost");
}

Client::~Client() {}

void Client::write(const std::string &msg)
{
	// send() may be used only when the socket is in a connected state (so that the intended recipient is known).  The
	// only difference between send() and write() is the presence of flags.  With a zero flags argument, send() is equivalent to write().
    if (send(sock_fd, msg.c_str(), msg.length(), 0) < 0)
        std::cerr << "Error while sending self message!" << std::endl;
}

std::vector<std::string> &Client::getChannels()
{
    std::vector<std::string> *channels = NULL;
    return *channels;
}


int Client::get_fd() const
{
    return (this->sock_fd);
}   
