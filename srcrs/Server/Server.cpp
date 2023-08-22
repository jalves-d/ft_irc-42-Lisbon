#include "Server.hpp"

Server::Server(int port, const char *password)
{
	this->port = port;
	this->password = password;
	this->server_sock = createSocket();
}

Server::~Server() {}

int Server::createSocket()
{
	//creating an endpoint for communication, socket() return a filedescriptor.
	//AF_INET is used to choose THE IPV4 Internet Protocol.
	//TCP (SOCK_STREAM) is a connection-based protocol. The connection is established and the two parties have a conversation until the connection is terminated by one of the parties or by a network error.
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		std::cout << "Socket creation failed!";
		exit(-1);
	}
	//setting the options to socket use the level specified to TCP comunicate using SOL_SOCKET.
	//SO_REUSEADDR is used to bind() be allowed to reuse local addresses.
	int options = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &options, sizeof(options)))
	{
		std::cout << "Socket options setting failed!";
		exit(-1);
	}
	//defining I/O operation as non-blocking, in this project is just allowed to use fcntl() using this flags as indicated on the subject.
	if (fcntl(sock, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cout <<"Sokcet flag options setting failed!";
		exit(-1);
	}
	//struct used to handle addresses.
	struct sockaddr_in server = {};
	server.sin_family = AF_INET;
    server.sin_port = htons(this->port);
	//INADDR_ANY running machine address.
    server.sin_addr.s_addr = INADDR_ANY;
    memset(&server.sin_zero, 0, 8);
	//Assigning the address specified by addr to the socket referred to by the file descriptor. Specifing the size in bytes of the address structure pointed to by addr.
	//Traditionally, this operation is called "assigning a name to a socket".
	if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        std::cout << "Binding socket operation failed!" << std::endl;
        exit(-1);
    }
	//Marking socket in connection-mode, specifying the socket as argument, as accepting connections.
	if (listen(sock, 100) < 0)
    {
        std::cout << "Listening on socket failed!." << std::endl;
        exit(-1);
    }
	return sock;
}

void Server::start()
{
	// Structure defined using the server socket file descriptor, POLLIN is used to indicate still having data to read.
    pollfd server_fd = {this->server_sock, POLLIN, 0};
    this->socket_poll.push_back(server_fd);
    std::cout << "Server listening on port: " << this->port << std::endl;
    while (true)
    {
		// Waiting until there is something on the poll
        if (poll(this->socket_poll.data(), this->socket_poll.size(), -1) < 0)
		{
            std::cout << "An error happened while polling !";
        }
        for (poll_iterator it = this->socket_poll.begin(); it != this->socket_poll.end(); it++)
		{
            if (it->revents == 0)
                continue;
			// This event indicates that still having data to read on the channel.
            if ((it->revents & POLLIN) == POLLIN)
			{
                if (it->fd == this->server_sock)
                {
                    std::cout << "client connected" << std::endl;
                    connectNewClient();
                    break;
                }
                if (newMessage(it->fd) == 0)
				{
					//std::cout << "Client disconnected" << std::endl;
					//disconnectClient(it->fd, getClient(), "client disconected");
					break;
				}
			}
			//This event indicates that the peer closed its end of the channel. Subsequent reads from the channel will return 0
			// only after all outstanding data in the channel has been consumed.
            if ((it->revents &POLLHUP) == POLLHUP)
			{
                std::cout << "client disconneted" << std::endl;
                disconnectClient(it->fd, getClient(), "client disconected");
                break;
            }
        }
    }
}

void Server::connectNewClient()
{
	//sockaddr_in describes an IPv4 Internet domain socket address.
	sockaddr_in temp;
    socklen_t len;
	int clientId;

    len = sizeof(temp);
	//accept() extracts the first connection request on the queue of pending connections for the listening socket, server_sock,
	//creates a new connected socket, and returns a new file descriptor referring to that socket.  The newly created socket
	//is not in the listening state.  The server_sock is unaffected by this call.
    clientId = accept(server_sock, (sockaddr *)&temp, &len);
    if (clientId < 0)
	{
        std::cerr << "An error ocurred while accepting new client !" << std::endl;
        exit(-1);
    }
	// Structure defined using the server socket file descriptor, POLLIN is used to indicate still having data to read.
    pollfd new_fd = {clientId, POLLIN, 0};
    socket_poll.push_back(new_fd);
    Client *newClient = new Client(clientId);
    std::string str = ":ft_irc 101 Welcome to ft_irc server!\r\n";
	newClient->flag = 1;
    newClient->write(str);
    clients.insert(std::make_pair(clientId, newClient));
}

int Server::newMessage(int sock_fd)
{
    std::string temp;
    char buffer[1000];
    this->temp_fd = sock_fd;
    while (true)
	{
        bzero(buffer, 1000);
		//recv is used to receive messages from a socket.
        int bytes = recv(sock_fd, buffer, 1000, 0);
        temp.append(buffer);
        if (bytes <= 0 || strchr(buffer, '\n') || strchr(buffer, '\r'))
            break;
    }
	if (temp.length() < 3)
        return 0;
    Message message = Message();
	std::cout << temp << std::endl;
    message.Message_picker(temp);
	if (!message.get_invalid())
		{executeCommands(message);
		return 1;}
	else
		std::cout << "Invalid message!" << std::endl;
	return 0;
}

void Server::executeCommands(Message &message)
{

	Client *client = getClient();
	std::string msg = message.get_command();
	std::cout << "Command: " << msg << std::endl;
	std::cout << "Compare: JOIN " << msg.compare("JOIN") << std::endl;
	if (msg.compare("JOIN") == 0)
		join(message.get_params(), *client);
	else if (msg.compare("LIST") == 0)
		list(message.get_params(), *client);
	else if (msg.compare("KICK") == 0)
		kick(message.get_params(), *client);
	else if (msg.compare("INVITE") == 0)
		invite(message.get_params(), *client);
	else if (msg.compare("MODE") == 0)
		mode(message.get_params(), *client);
	else if (msg.compare("TOPIC") == 0)
		topic(message.get_params(), *client);
	else if (msg.compare("NICK") == 0)
		nick(message.get_params(), *client);
	else if (msg.compare("QUIT") == 0)
		quit(message.get_params(), *client);
	else if (msg.compare("PRIVMSG") == 0)
		privmsg(message.get_params(), *client);
	else
		std::cout << "Command invalid" << std::endl;
}

void Server::disconnectClient(int sock, Client *client, std::string msg)
{
    std::vector<std::string> userChannels;
    std::vector<Channel*> serverChannels;
    Server::poll_iterator it;
    Server::channel_iterator ch_it;
    Channel::channellUsersIt tic;

    if (client != NULL)
	{
        userChannels = client->getChannels();
        serverChannels = getChannels();
        for(it = getPoll().begin(); it != getPoll().end(); it++)
		{
            if (it->fd == sock)
                break;
        }
        if (it != getPoll().end())
		{
            getPoll().erase(it);
            for(ch_it = serverChannels.begin(); ch_it != serverChannels.end(); ch_it++)
			{
                if (std::find(userChannels.begin(), userChannels.end(), (*ch_it)->channelName) != userChannels.end())
				{
                    notifyAllClients((*ch_it), *client, msg);
                    for(tic = (*ch_it)->channelUsers.begin(); tic != (*ch_it)->channelUsers.end(); tic++)
					{
                        if (client->nickname == (*tic).first->nickname)
                            {(*ch_it)->channelUsers.erase(tic);
                            return;}
                    }
                }
            }
            close(client->sock_fd);
        }
    }
}

void Server::notifyAllClients(Channel const *channel, Client &client, std::string msg)
{
    std::map<Client*, int> users;
    Channel::channellUsersIt it;
    std::string message;

    users = channel->channelUsers;
    for (it = users.begin(); it != users.end(); it++)
	{
        if ((it)->first->nickname != client.nickname)
		{
            message = "* " + client.nickname + " has quit (" + msg +")\r\n";
            (it)->first->write(message);
        }
    }
}

Channel &Server::getChannel(std::string &channelSName)
{
    unsigned long i;

    for (i = 0; i < channels.size(); i++) {
        if (channels[i]->channelName == channelSName)
            return *channels[i];
    }
    return(*channels[i]);
}

void Server::kick(std::string cmd, Client &client)
{
	std::stringstream cmds(cmd);
	std::string move;
	Channel *channel = NULL;
	Channel::channellUsersIt    it;

	cmds >> move;
	if (move.empty())
	{
		std::cout << "Invalid channel Name!" << std::endl;
		return;
	}
	else
		*channel = getChannel(move);
	if (channel == NULL)
	{
		std::cout << "Invalid channel Name!" << std::endl;
		return;
	}
	else if (!channel->verifyAdminPrivilege(client.nickname))
	{
		std::cout << "You dont have privileges to do this operation!" << std::endl;
		return;
	}
	else
	{
		cmds >> move;
		for(it = channel->channelUsers.begin(); it != channel->channelUsers.end(); it++)
		{
            if (move == (*it).first->username)
			{
                channel->channelUsers.erase(it);
				for(it = channel->channelUsers.begin(); it != channel->channelUsers.end(); it++)
        			(*it).first->write(":" + (&client)->nickname + " KICK " + channel->channelName + " " + move + " :" + "KICKED\r\n");
					//if necessary change move to nickname
				return;
            }
		}
	}
	std::cout << "Has no user using this nickname on the channel!" << std::endl;
}

int verifyUserInChannel(std::string move, Channel &channel)
{
	Channel::channellUsersIt    it;
	for(it = (&channel)->channelUsers.begin(); it != (&channel)->channelUsers.end(); it++)
	{
        if (move == (*it).first->nickname)
			return 1;
	}
	return 0;
}

void Server::invite(std::string cmd, Client &client)
{
	std::stringstream cmds(cmd);
	std::string move;
	std::string usnickname;
	Channel *channel = NULL;
	Channel::channellUsersIt    it;
	Server::client_iterator cit;
	std::map<int, Client*> serverCli = getClients();

	cmds >> usnickname;
	cmds >> move;
	if (move.empty())
	{
		std::cout << "Invalid channel Name!" << std::endl;
		return;
	}
	else
		*channel = getChannel(move);
	if (channel == NULL)
	{
		std::cout << "Invalid channel Name!" << std::endl;
		return;
	}
	else if (!channel->verifyAdminPrivilege((&client)->nickname))
	{
		std::cout << "You dont have privileges to do this operation!" << std::endl;
		return;
	}
	else
	{
		for(cit = serverCli.begin(); cit != serverCli.end(); cit++)
		{
            if (usnickname == (*cit).second->nickname)
			{
				if (verifyUserInChannel(usnickname, *channel))
				{
					std::cout << "The user is already part of this channel!" << std::endl;
					return;
				}
                channel->channelUsers.insert(std::make_pair((*cit).second, 0));
				for (it = channel->channelUsers.begin(); it != channel->channelUsers.end(); it++)
        			(*it).first->write(":" + client.nickname + " INSERT " + channel->channelName + " " + usnickname + " :" + "INSERTED\r\n");
				return;
            }
		}
	}
	std::cout << "Has no user using that nickname on the server!" << std::endl;
}

void Server::topic(std::string cmd, Client &client)
{
	std::stringstream cmds(cmd);
	std::string move;
	Channel *channel = NULL;
	Channel::channellUsersIt    it;
	Channel::channellUsersIt    sit;

	cmds >> move;
	if (move.empty())
	{
		std::cout << "Invalid channel Name!" << std::endl;
		return;
	}
	else
		*channel = getChannel(move);
	if (channel == NULL)
	{
		std::cout << "Invalid channel Name!" << std::endl;
		return;
	}
	else if (!channel->verifyUserInChannel((&client)->nickname))
	{
		std::cout << "You aren't a member of this channel!" << std::endl;
		return;
	}
	else if (channel->adminOnlyTopic == true && !channel->verifyAdminPrivilege((&client)->nickname))
	{
		std::cout << "You dont have privileges to do this operation!" << std::endl;
		return;
	}
	else
	{
		cmds >> move;
		for(it = channel->channelUsers.begin(); it != channel->channelUsers.end(); it++)
		{
			//Atualizar isso, setChannel atualizada, adicionar uma string temporaria.
        	(*it).first->write(":" + client.nickname + " TOPIC " + channel->channelName + " " + move + " :" + "CHANGED\r\n");
			if (channel->setChannel(move, (*this)))
				return;
		}
	}
	std::cout << "Has no user using this nickname on the channel!" << std::endl;
}

void Server::mode(std::string cmd, Client &client)
{
	std::stringstream cmds(cmd);
	std::string move;
	Channel *channel = NULL;
	Channel::channellUsersIt    it;
	Channel::channellUsersIt    sit;

	cmds >> move;
	if (move.empty())
	{
		std::cout << "Invalid channel Name!" << std::endl;
		return;
	}
	else
		*channel = getChannel(move);
	if (channel == NULL)
	{
		std::cout << "Invalid channel Name!" << std::endl;
		return;
	}
	else if (!channel->verifyAdminPrivilege((&client)->nickname))
	{
		std::cout << "You dont have privileges to do this operation!" << std::endl;
		return;
	}
	else
	{
		std::string more;
		cmds >> move;
		if (move.empty())
		{
			std::cout << "Missing flags on command MODE!"<<std::endl;
			return;
		}
		cmds >> more;
		if (!more.empty() && !(move[1].c_str() == 'o' || move[1].c_str() == 'l' || move[1].c_str() == 'k'))
		{
			std::cout << "Too many arguments you just can set one include or one exclude per command!"<<std::endl;
			return;
		}
		if (more.empty() && (move[1].c_str() == 'o' || move[1].c_str() == 'l' || move[1].c_str() == 'k'))
		{
			std::cout << "Missing argument after flag!"<<std::endl;
			return;
		}
		int i = 1;
		int mode = -1;
		if (move.c_str()[0] == '+')
				mode = 0;
		else if (move.c_str()[0] == '-')
				mode = 1;
		else
		{
			std::cout << "Missing flags on command MODE!"<<std::endl;
			return;
		}
		if (move.c_str()[1] == 'o' && mode == 0)
			channel->changeAdminPrivilege(more, true);
		else if (move.c_str()[1] == 'o' && mode == 1)
			channel->changeAdminPrivilege(more, false);
		else if (move.c_str()[1] == 'i' && mode == 0)
			channel->changeInviteOnly(true);
		else if (move.c_str()[1] == 'i' && mode == 1)
			channel->changeInviteOnly(false);
		else if (move.c_str()[1] == 't' && mode == 0)
			channel->changeAOT(true);
		else if (move.c_str()[1] == 't' && mode == 1)
			channel->changeAOT(false);
		else if (move.c_str()[1] == 'k' && mode == 0)
			channel->setPassword(more);
		else if (move.c_str()[1] == 'k' && mode == 1)
			channel->setPassword(NULL);
		else if (move.c_str()[1] == 'l' && mode == 1)
			channel->setUsersLimit(-1); //error verification need to be implemented
		else if (move.c_str()[1] == 'l' && mode == 0)
			channel->setUsersLimit(std::atoi(more.c_str()));
		std::cout << "Mode set with success!" << std::endl;
		return;
	}
	std::cout << "Has no user using this nickname on the channel!" << std::endl;
}

void Server::list(std::string cmd, Client &client)
{
	std::vector<Channel*> schannels = getChannels();
	channel_iterator cit;
	int cont = 0;
	client.write("Channel list:\n");
	if (cmd.empty())
	{
		for (cit = schannels.begin(); cit != schannels.end(); cit++)
		{
			cont++;
			client.write((*cit)->channelName + to_string(cont) + " (" + to_string((*cit)->channelUsers.size()) + ") - Topic: " + (*cit)->topic + "\n");
		}
	}
	else
	{
		std::stringstream cmds(cmd);
		std::string move;
		cmds >> move;
		for (cit = schannels.begin(); cit != schannels.end(); cit++)
		{
			cont++;
			if ((*cit)->channelName.find(move) != std::string::npos)
				client.write((*cit)->channelName + to_string(cont) + " (" + to_string((*cit)->channelUsers.size()) + ") - Topic: " + (*cit)->topic + "\n");
		}
	}
}

void Server::join(std::string cmd, Client &client)
{
	std::stringstream cmds(cmd);
	std::string move;
	Channel *channel = NULL;

	cmds >> move;
	if (move.empty())
	{
		std::cout << "Invalid channel Name!" << std::endl;
		return;
	}
	else
		*channel = getChannel(move);
	if (channel == NULL)
	{
		*channel = new Channel(move, client);
		this->channels.insert(channel);
		std::cout << "New Channel Created!" << std::endl;
		return;
	}
	else if (channel->verifyUserInChannel((&client)->nickname))
	{
		std::cout << "You are already a member of this channel!" << std::endl;
		return;
	}
	else if (channel->inviteOnly)
	{
		std::cout << "This channel is set as invite only!" << std::endl;
		return;
	}
	cmds >> move;
	if (channel->password == move)
	{
		channel->channelUsers.insert(client, 1);
		std::cout << "Client joined the channel!" << std::endl;
		return;
	}
	//implement notify all clients in channel
	std::cout << "Incorrect Password !" << std::endl;
}

void Server::nick(std::string cmd, Client &client)
{
	std::stringstream cmds(cmd);
	std::string move;
	Server::client_iterator cit;
	std::map<int, Client*> serverCli = getClients();

	cmds >> move;
	if (move == NULL)
	{
		std::cout << "No nickname provided!" << std::endl;
		return;
	}
	if (client.nickname.compare(move) == 0)
	{
		std::cout << "The nickname provided is already your current nickname!" << std::endl;
		return;
	}
	for(cit = serverCli.begin(); cit != serverCli.end(); cit++)
	{
        if (usnickname == (*cit).second->nickname)
		{
			std::cout << "The nickname provided is already in use!" << std::endl;
			return;
		}
	}
	client.nickname = move;
	std::cout << "Your new nickname is " + move << std::endl;
}

void Server::quit(std::string cmd, Client &client)
{
	std::stringstream cmds(cmd);
	std::string move;
	Channel *channel = NULL;
	Channel::channellUsersIt    it;

	cmds >> move;
	if (move.empty())
	{
		std::cout << "Invalid channel Name!" << std::endl;
		return;
	}
	else
		*channel = getChannel(move);
	if (channel == NULL)
	{
		std::cout << "Invalid channel Name!" << std::endl;
		return;
	}
	else if (!channel->verifyUserInChannel(client.nickname))
	{
		std::cout << "You are not a member of this channel!" << std::endl;
		return;
	}
	else
	{
		for(it = channel->channelUsers.begin(); it != channel->channelUsers.end(); it++)
		{
            if (client.username == (*it).first->username)
			{
                channel->channelUsers.erase(it);
				for(it = channel->channelUsers.begin(); it != channel->channelUsers.end(); it++)
        			(*it).first->write(":" + (&client)->nickname + " QUIT " + channel->channelName + " " + move + " :" + "QUIT\r\n");
					//if necessary change move to nickname
				return;
            }
		}
	}
	std::cout << "Has no user using this nickname on the channel!" << std::endl;
}

void Server::privmsg(std::string cmd, Client &client)
{
	(void)cmd;
	(void)client;
}
