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
    if (channelUsers[clientFd] == 1)
        return true;
    return false;
}
bool Channel::verifyUserInChannel(int clientFd){
    if (channelUsers[clientFd] == 0 || channelUsers[clientFd] == 1)
        return true;
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
    if (channelUsers[client.sock_fd] == 1){
        std::string str = ":local 443 " + client.nickname + " " + this->channelName + " :You are already in " + this->channelName + "\r\n";
        client.write(str);
        return false;}
    if (channelUsers[client.sock_fd] == 0){
        std::string str = ":local 443 " + client.nickname  + " " +this->channelName + " :You are already in " + this->channelName + "\r\n";
        client.write(str);
        return false;}
    if (this->hasUserLimit != 0 && this->channelUsers.size() >= this->usersLimit){
        std::string str = ";local 471 " + client.nickname + " " + this->channelName + " :Channel " + this->channelName + " is full (-l))\r\n";
        return false;}
    if (this->hasPassword != 0 && this->password.compare(pass) != 0){
        std::string str = ":local 475 " + client.nickname + " " + this->channelName + " :Cannot join channel (+k)\r\n";
        client.write(str);
        return false;
    }

    channelUsers.insert(std::pair<int, int>(client.sock_fd, 0));
    return true;
}

bool Channel::removeClient(Client &client)
{
    if (channelUsers[client.sock_fd] == 1){
        channelUsers.erase(client.sock_fd);
        return true;
    }
    if (channelUsers[client.sock_fd] == 0){
        channelUsers.erase(client.sock_fd);
        return true;
    }
    return false;
}

bool Channel::removeClient(Client &client){ //remove a client from the channel
    //add message to the channel
    if (this->channelUsers[client.sock_fd] == 1){
        channelUsers.erase(client.sock_fd);
        return true;
    }
    if (channelUsers[client.sock_fd] == 0){
        channelUsers.erase(client.sock_fd);
        return true;
    }
    return false;
}

bool Channel::kickClient(Client &client, Client &kicker){ //kick a client from the channel, fd is the kicking users fd
    if (channelUsers[kicker.get_fd()] == 1){
        if (removeClient(client) == false){
            std::string str = ":local 401 " + kicker.nickname + " " + this->channelName + " " + client.nickname + " :No such target\r\n";
            client.write(str);
            return false;
        }
        //:kickerNickname!kickerUsername@kickerHost KICK #channel kickedNickname :reason
        if (this->channelUsers[client.sock_fd] == 1){
            channelUsers.erase(client.sock_fd);
            return true;
        }
        if (channelUsers[client.sock_fd] == 0){
            channelUsers.erase(client.sock_fd);
            return true;
        }
            return true;
    }
    return false;
}        