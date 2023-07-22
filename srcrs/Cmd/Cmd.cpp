#include "Cmd.hpp"

Cmd::Cmd(const char *) { }

Cmd::Cmd()
{
    this->command = "";
}

Cmd::Cmd(string &cmd)
{
    this->command = cmd;
}

Cmd::~Cmd() {}


void quit(std::string str, Client &client, Server &server)
{
    server.disconnectClient(client.sock_fd, &client, str);
}

void kick()