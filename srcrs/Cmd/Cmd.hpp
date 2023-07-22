#ifndef CMD_HPP
# define CMD_HPP

# include "string.h"

# include "../Server/Server.hpp"
# include "../Client/Client.hpp"
# include "../Channel/Channel.hpp"


class Cmd {
    private:
		bool invalid;
		std::string cmd;
		std::string arg;
		std::string usr;
		bool got_user;
		//Client user;
        //std::string command;
    public:
		Cmd();
		~Cmd();

		//Channel Commands
		Cmd cmd_picker(std::string message);



		/*void kick(std::vector<std::string> &, Client &, Server &);
		void invite(std::vector<std::string>&, Client &, Server&);
		void topic(std::vector<std::string> &, Client &, Server &);
		void mode(std::vector<std::string>&, Client &, Server &);*/
};

#endif
