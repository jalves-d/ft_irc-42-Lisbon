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
