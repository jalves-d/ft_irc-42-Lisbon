#include "Client.hpp"

Client::Client() {}

Client::Client(int fd)
{
    this->sock_fd = fd;
    this->avail = 0;
}

Client::Client(Client const &clt)
{
    *this = clt;
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
    return channels;
}
