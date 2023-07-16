#ifndef CMD_HPP
# define CMD_HPP

#include "../Server/Server.hpp"
#include "../Client/Client.hpp"
#include "../Channel/Channel.hpp"

class Server;

class Cmd {
    private:
        std::string command;
    public:
		Cmd();
		Cmd(const char *);
        Cmd(std::string &);
		~Cmd();

		//Channel Commands
		void kick(std::vector<std::string> &, Client &, Server &);
		void invite(std::vector<std::string>&, Client &, Server&);
		void topic(std::vector<std::string> &, Client &, Server &);
		void mode(std::vector<std::string>&, Client &, Server &);
};

#endif
