#include "Message.hpp"

Message::Message() {
    this->invalid = true;
    this->got_prefix = false;
    this->command = "";
    this->params = "";
    this->prefix = "";
}

Message::~Message() {}

Message::Message(Message const &src){
    *this = src;
}
//function that takes the string sent by the client and seperates it into its parts
void Message::Message_picker(std::string message) {
    std::vector<std::string> words;
    this->got_prefix = false;

    size_t start = 0, end;

    while ((end = message.find(' ', start)) != std::string::npos) {
        words.push_back(message.substr(start, end - start));
        start = end + 1;
    }
    words.push_back(message.substr(start));

    if (words.empty()) {
        this->invalid = true;
        return;
    }

    if (words.size() < 2) {
        this->invalid = true;
        return;
    }

    unsigned int i = 0;
    if (!words[i].empty() && words[i][0] == ':') {
        if (words[i].size() == 1 || words[i][1] == ' ') {
            this->invalid = true;
            return;
        }
        this->got_prefix = true;
        this->prefix = words[i].substr(1);
        i++;
    }

    if (words[i].empty()) {
        this->invalid = true;
        return;
    }
    
    this->command = words[i];
    i++;

    while (i < words.size()) {
        this->params += ' ' + words[i];
        i++;
    }
    this->params.erase(0, 1);
    this->invalid = false;

    /*std::cout << "prefix: " << this->prefix << std::endl;
    std::cout << "command: " << this->command << std::endl;
    std::cout << "params: " << this->params << std::endl;*/
}


std::string Message::get_command(void)
{
    return(this->command);
}
std::string Message::get_params(void){
    return(this->params);
}

std::string Message::get_prefix(void){
    return(this->prefix);
}

bool Message::get_got_prefix(void){
    return(this->got_prefix);
}

bool Message::get_invalid(void){
    return(this->invalid);
}


