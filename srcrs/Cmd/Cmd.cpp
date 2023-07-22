#include "Cmd.hpp"

Cmd::Cmd() {}

Cmd::~Cmd() {}

//function that takes the string sent by the client and seperates it into its parts and selects what funtions to call based on that
Cmd Cmd::cmd_picker(std::string message){
    Cmd request;
    int i = 0;
    int j = 0;
    request.got_user = false;
    std::stringstream stream(message);
    std::string word;
    if (message[i]  == ' ' || !message[i])
    {
        request.invalid = true;
        return (request);
    }
    
    stream >> word;
    if (!word[0])
    {
        request.invalid = true;
        return (request);
    }
    if (word[0] == ':')
    {
        if(word[1] == ' ' || word[1] == 0)
        {
            request.invalid = true;
            return (request);
        }
        request.got_user =  true;
        request.usr = word;
    }

    if (request.got_user)
    {
        stream >> word;
        if (!word[0])
        {
        request.invalid = true;
        return (request);
        }
    }
    request.cmd = word;
    while (stream >> word);
    {
        request.arg = request.arg + ' ' + word;
    }
    request.invalid = false;
    return(request);
}

