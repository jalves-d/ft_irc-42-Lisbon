#ifndef CMD_HPP
# define CMD_HPP

# include <string>
# include <sstream>
# include <iostream>
# include <vector>

class Cmd {
    private:
		bool invalid;
		std::string command;
		std::string params;
		std::string prefix;
		bool got_prefix;

    public:
		Cmd();
		~Cmd();	
		void cmd_picker(std::string message);
		std::string get_command(void);
		std::string get_params(void);
		std::string get_prefix(void);
		bool get_got_prefix(void);
		bool get_invalid(void);
};

#endif
