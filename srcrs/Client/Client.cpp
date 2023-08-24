#include "Client.hpp"

Client::Client() {}

Client::Client(int fd)
{
    this->sock_fd = fd;
    this->avail = 0;
    this->disconected = false;
    this->registered_pass = false;
    this->registered_nick = false;
    this->registered_user = false;
    this->registered_full = false;
    //std::cout << "New client created. (temp)FD: " << fd << std::endl;
}

Client::~Client(){
    //std::cout << "Client destroyed. FD: " << this->sock_fd << std::endl;
}

Client::Client(Client const &clt)
{
    this->sock_fd = clt.sock_fd;
    this->nickname = clt.nickname;
    this->username = clt.username;
    this->mod = clt.mod;
    this->avail = clt.avail;
    this->flag = clt.flag;
    this->disconected = clt.disconected;
    this->registered_pass = clt.registered_pass;
    this->registered_nick = clt.registered_nick;
    this->registered_user = clt.registered_user;
    this->registered_full = clt.registered_full;
    //std::cout << "New client created. FD: " << clt.sock_fd << std::endl;
}

Client &Client::operator=(const Client &clt) {
    if (this == &clt)
        return *this;
    // Copy member values here
    this->sock_fd = clt.sock_fd;
    this->nickname = clt.nickname;
    this->username = clt.username;
    this->mod = clt.mod;
    this->avail = clt.avail;
    this->flag = clt.flag;
    this->disconected = clt.disconected;
    this->registered_pass = clt.registered_pass;
    this->registered_nick = clt.registered_nick;
    this->registered_user = clt.registered_user;
    this->registered_full = clt.registered_full;
    return *this;
}


int Client::available() { return avail; }

std::string Client::getPrefix() const
{
	return this->nickname + ("!" + this->username) + ("@localhost");
}


void Client::write(const std::string &msg)
{
	// send() may be used only when the socket is in a connected state (so that the intended recipient is known).  The
	// only difference between send() and write() is the presence of flags.  With a zero flags argument, send() is equivalent to write().
    std::string message = msg + "\r\n";
    if (send(sock_fd, message.c_str(), message.length(), 0) < 0)
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

void Client::setDisconnected(bool b)
{
    this->disconected = b;
}

bool Client::isDisconnected() 
{
    return this->disconected;
}