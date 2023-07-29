#include "Cmd.hpp"

Cmd::Cmd() {}

Cmd::~Cmd() {}

//function that takes the string sent by the client and seperates it into its parts
void Cmd::cmd_picker(std::string message){

    std::vector<std::string> words;
    this->got_prefix =  false;
    int end = message.find(' ');
    if (end == 0){
        this->invalid = true;
        return;
    }

    while (end != -1){
        words.push_back(message.substr(0, end));
        message.erase(message.begin(), message.begin() + end + 1);
        end = message.find(' ');
    }
    words.push_back(message.substr(0, end));
    int i = 0;

    if (words[i][0] == ':'){
        if (words[i][1] == ' ' || words [i][1] == 0){
            this->invalid = true;
            return;
        }
        this->got_prefix = true;
        this->prefix = words[i];
        i++;
    }

    if (!words[i][0]){
        this->invalid = true;
        return;
    }
    this->command = words[i];
    i++;
    while (i <= 1 + words.size())
    {
        this->params = this->params + ' ' + words[i];
        i++;
    }
    this->invalid = false;
    this->params.erase(this->params.begin(), this->params.begin() + 1);

} 


std::string Cmd::get_command(void)
{
    return(this->command);
}
std::string Cmd::get_params(void){
    return(this->params);
}

std::string Cmd::get_prefix(void){
    return(this->prefix);
}

bool Cmd::get_got_prefix(void){
    return(this->got_prefix);
}

bool Cmd::get_invalid(void){
    return(this->invalid);
}


