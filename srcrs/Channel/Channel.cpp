#include "Channel.hpp"

Channel::Channel(Channel const &og)
{
    this->channelName = og.channelName;
    this->channelUsers = og.channelUsers;
    this->topic = og.topic;
    this->password = og.password;
    this->adminOnlyTopic = og.adminOnlyTopic;
    this->usersLimit = og.usersLimit;
    this->inviteOnly = og.inviteOnly;
    this->hasUserLimit = og.hasUserLimit;
    this->hasPassword = og.hasPassword;
    this->hasTopic = og.hasTopic;
}

Channel &Channel::operator=(Channel const &channel)
{
    if (this == &channel)
        return *this;
    // Copy member values here
    this->channelName = channel.channelName;
    this->channelUsers = channel.channelUsers;
    this->topic = channel.topic;
    this->password = channel.password;
    this->adminOnlyTopic = channel.adminOnlyTopic;
    this->usersLimit = channel.usersLimit;
    this->inviteOnly = channel.inviteOnly;
    this->hasUserLimit = channel.hasUserLimit;
    this->hasPassword = channel.hasPassword;
    this->hasTopic = channel.hasTopic;
    return *this;
}

Channel::Channel()
{
    this->channelName = "";
    this->topic = "";
    this->password = "";
    this->adminOnlyTopic = false;
    this->usersLimit = 0;
    this->inviteOnly = false;
    this->hasUserLimit = false;
    this->hasPassword = false;
    this->hasTopic = false;
}

Channel::Channel(std::string name, Client &client)
{
    this->channelName = name;
    this->topic = "";
    this->password = "";
    this->adminOnlyTopic = false;
    this->usersLimit = 0;
    this->inviteOnly = false;
    this->hasUserLimit = false;
    this->hasPassword = false;
    this->hasTopic = false;
    channelUsers.insert(std::pair<int, int>(client.sock_fd, 1));
}

Channel::~Channel() {}

bool Channel::verifyAdminPrivilege(int clientFd){
    std::map<int, int>::iterator it = channelUsers.find(clientFd);

    if (it != channelUsers.end() && (it->second == 1)) {
        return true;
    }

    return false;
}

bool Channel::verifyUserInChannel(int clientFd) {
    std::map<int, int>::iterator it = channelUsers.find(clientFd);

    if (it != channelUsers.end() && (it->second == 0 || it->second == 1)) {
        return true;
    }

    return false;
}

bool Channel::addAdminPrivilege(int to_change_clientFd, int request_clientFd){
    if (channelUsers[request_clientFd] == 1){
        if (channelUsers[to_change_clientFd] == 0){
            channelUsers[to_change_clientFd] = 1;
            return true;
        }
        else if (channelUsers[to_change_clientFd] == 1){
            return false;
        }
    }
    return false;
}

bool Channel::removeAdminPrivilege(int to_change_clientFd, int request_clientFd){
    if(channelUsers[request_clientFd] == 1){
        if (channelUsers[to_change_clientFd] == 1){
            channelUsers[to_change_clientFd] = 0;
            return true;
        }
        else if (channelUsers[to_change_clientFd] == 0){
            return false;
        }
    }
    return false;
}

bool Channel::setChannelTopic(std::string topic, int fd){
    if (channelUsers[fd] == 1){
        this->topic = topic;
        return true;
    }
    if (channelUsers[fd] == 0 && !this->adminOnlyTopic){
        this->topic = topic;
        return true;
    }
    return false;
}
bool Channel::setUsersLimit(int limit, int fd){
    if (channelUsers[fd] == 1){
        this->hasUserLimit = true;
        this->usersLimit = limit;
        return true;
    }
    return false;
}

bool Channel::removeUsersLimit(int fd){
    if (channelUsers[fd] == 1){
        this->hasUserLimit = false;
        this->usersLimit = 0;
        return true;
    }
    return false;
}
bool Channel::setPassword(std::string pass, int fd) {
    if (channelUsers[fd] == 1){
        this->hasPassword = true;
        this->password = pass;
        return true;
    }
    return false;
}

bool Channel::removePassword(int fd) {
    if (channelUsers[fd] == 1){
        this->hasPassword = false;
        this->password = "";
        return true;
    }
    return false;
}
bool Channel::changeAOT(bool ad, int fd){
    if(channelUsers[fd] == 1){
        if (this->adminOnlyTopic == ad)
            return false;
        this->adminOnlyTopic = ad;
        return true;
    }
    return false;
}
bool Channel::changeInviteOnly(bool ad, int fd){
    if(channelUsers[fd] == 1){
        if (this->inviteOnly == ad)
            return false;
        this->inviteOnly = ad;
        return true;
    }
    return false;
}

bool Channel::addClient(Client &client, std::string pass)
{
    std::cout << "addClient. FD = " << client.sock_fd << std::endl;
    std::map<int, int>::iterator it;
    for (it = channelUsers.begin(); it != channelUsers.end(); it++)
        std::cout << "FD: " << it->first << " | " << it->second << std::endl;
    

    if (verifyUserInChannel(client.sock_fd) == true){
        std::string str = ":local 443 " + client.nickname + " " + this->channelName + " :You are already in " + this->channelName + "\r\n";
        client.write(str);
        return false;}
    
    else if (this->hasUserLimit != 0 && this->channelUsers.size() >= this->usersLimit){
        std::string str = ":local 471 " + client.nickname + " " + this->channelName + " :Channel " + this->channelName + " is full (-l))\r\n";
        client.write(str);
        return false;}
    else if (this->hasPassword != 0 && this->password.compare(pass) != 0){
        std::string str = ":local 475 " + client.nickname + " " + this->channelName + " :Cannot join channel (+k)\r\n";
        client.write(str);
        return false;
    }
    else{
    channelUsers.insert(std::pair<int, int>(client.sock_fd, 0));
    std::string str = ":server 332 " + client.nickname + " " + this->channelName + " :Welcome to the channel " + this->channelName;
    client.write(str);

    str = ":" + client.getPrefix() + " JOIN " + this->channelName;
    client.write(str);

    return true;
    }
}

bool Channel::removeClient(Client &client){ //remove a client from the channel
    //add message to the channel
    if (this->verifyUserInChannel(client.sock_fd) == true){
        channelUsers.erase(client.sock_fd);
        return true;
    }
    return false;
}

bool Channel::kickClient(Client &client, Client &kicker){ //kick a client from the channel, fd is the kicking users fd
    if (channelUsers[kicker.get_fd()] == 1){
        if (removeClient(client) == false){
            std::string str = ":local 401 " + kicker.nickname + " " + this->channelName + " " + client.nickname + " :No such target";
            kicker.write(str);
            return false;
        }
        return true ; 
    }
    std::string str = ":local 482 " + kicker.nickname + " " + this->channelName + " :You're not channel operator";
    return false;
}


std::string Channel::returnModes()
{
    std::string str = "+";
    if (this->adminOnlyTopic == true)
        str += "t";
    if (this->inviteOnly == true)
        str += "i";
    if (this->hasPassword == true)
        str += "k";
    if (this->hasUserLimit == true)
        str += "l";
    return str;
}