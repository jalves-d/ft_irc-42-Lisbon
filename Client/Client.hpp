#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <iostream>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <map>
#include <sys/poll.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <algorithm>
#include <sstream>
#include <poll.h>
#include <cstdio>
#include <functional>
#include <cctype>
#include <locale>

class Client {
    public:
        string nickname;
        string username;
        string password;
        vector<string> channels;
        int mod;
        int sock_fd;
		int available;
        int flag;

        Client();
        Client(int);
        Client(Client const&);
        ~Client();

        Client &operator=(Client const &clt);
        void write(const std::string &);
        std::string getPrefix() const;
		int available();
        std::vector<std::string> &getChannels();
};

#endif
