#include "Cmd.hpp"

Cmd::Cmd(const char *) { }

Cmd::Cmd()
{
    this->command = "";
}

Cmd::Cmd(string &cmd, Client client):   User(client)
{
    this->command = cmd;
}

Cmd::~Cmd() {}

//function that takes the string sent by the client and seperates it into its parts and selects what funtions to call based on that
void Cmd::cmd_picker(std::string message){

}