#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include "../Server/Server.hpp"


class Client;
class Server;

class Channel
{
    public:
        std::string channelName; //name of the channel
        std::map<int,int> channelUsers;//used to store the fd of the user and the mode of the user (0 = normal, 1 = op)
		std::string topic; //topic of the channel
        std::string password; //password of the channel
		bool adminOnlyTopic; //if the topic can be changed only by the admin
		int usersLimit; //limit of users in the channel
		bool inviteOnly; //if the channel is invite only
        bool hasUserLimit; //if the channel has a user limit
        bool hasPassword; //if the channel has a password
        bool hasTopic; //if the channel has a topic

		Channel(Channel const&);
		Channel &operator=(Channel const &channel);
		Channel();
		Channel(std::string, Client&);
		~Channel();
		bool verifyAdminPrivilege(int fd);
		bool verifyUserInChannel(int fd);
		bool addAdminPrivilege(int fd, int);//change the admin privilege of a user. return true if ok, false if not. first fd is the fd of the user to change, second fd is the fd of the user that wants to change the privilege
		bool removeAdminPrivilege(int fd, int);//change the admin privilege of a user. return true if ok, false if not. first fd is the fd of the user to change, second fd is the fd of the user that wants to change the privilege
        bool setChannelTopic(std::string topic, int fd);
		bool setUsersLimit(int limit, int fd);
        bool removeUsersLimit(int fd);
	    bool setPassword( std::string pass, int fd);
        bool removePassword(int fd);
		bool changeAOT(bool ad, int fd);
	    bool changeInviteOnly(bool ad, int fd);

        bool addClient(Client &client, std::string pass); //add a client to the channel
        bool removeClient(Client &client); //remove a client from the channel
        bool kickClient(Client &client, Client &kicker); //kick a client from the channel, kicker is who is trying to kick
        

};

#endif