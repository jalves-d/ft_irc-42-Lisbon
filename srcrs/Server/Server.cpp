#include "Server.hpp"


Server::Server(int port, const char *password)
{
	this->port = port;
	this->password = password;
    this->hostname = "local";
	epollfd = epoll_create1(0); // Initialize epollfd
    if (epollfd == -1) {
        perror("epoll_create1");
        return;
    }
}

Server::~Server() {}

void Server::start() {
	const int MAX_EVENTS = 100000;
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("socket");

        exit(-1);
    }
    int flags = fcntl(serverSocket, F_GETFL, 0);
    fcntl(serverSocket, F_SETFL, flags | O_NONBLOCK);
    struct sockaddr_in serverAddr = {};;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);  // Use the provided port
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("bind");
        std::cout << "Port: " << port << std::endl;

        exit(-1);
    }

    if (listen(serverSocket, 5) == -1) {
        perror("listen");
        exit(-1);
    }


    struct epoll_event event;
    event.data.fd = serverSocket;
    event.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, serverSocket, &event) == -1) {
        perror("epoll_ctl");
        return;
    }

    std::vector<struct epoll_event> events(MAX_EVENTS);
    std::cout << "Server started on port " << port << std::endl;
    while (true) {
        int numEvents = epoll_wait(epollfd, &events[0], MAX_EVENTS, -1);
        for (int i = 0; i < numEvents; ++i) {
            if (events[i].data.fd == serverSocket) {
                int clientSocket = accept(serverSocket, NULL, NULL);
                newClient(clientSocket);
            } else {
                if (events[i].events & EPOLLHUP) {
                    std::cout << "Client disconnected in EPOLLHUP: " << events[i].data.fd << std::endl;
                    handleDisconnect(events[i].data.fd);
                    }
                else if (events[i].events & EPOLLIN) {
                    newMessage(events[i].data.fd);}
                    // Handle data received from clients
                    // Similar to the initial example
                else
                {
                    std::cout << "Unknown event" << std::endl;
                }
            }
            for (std::list<Client>::iterator it = clients.begin(); it != clients.end();) {
                if ((it->isDisconnected()) == true) {
                    std::cout << "Client removed from list: " << it->get_fd() << std::endl;
                    it = clients.erase(it);
                    } else {
                        ++it;
                    }
            }
        }
    }   

    close(epollfd);
    close(serverSocket);
}

void Server::newMessage(int clientSocket) {
    char buffer[1024]; // Adjust the buffer size as needed
    ssize_t bytesRead = recv(clientSocket, &buffer, sizeof(buffer) - 1, 0);
    std::cout << "Bytes read: " << bytesRead << std::endl;
 // Map to store client message buffers
    if (bytesRead == -1) {
        perror("recv");
        return;
    } else if (bytesRead == 0) {
        // Client disconnected    
        handleDisconnect(clientSocket);
        std::cout << "Client sent empty message: " << clientSocket << std::endl;
    } else {
        buffer[bytesRead] = '\0'; // Null-terminate the received data
        std::string str_buffer(buffer);
        while (str_buffer.find("\r\n") != std::string::npos){
            std::string str = client_buffers[clientSocket] + str_buffer.substr(0, str_buffer.find("\r\n"));
            client_buffers[clientSocket] = "";
            std::cout << "Received message from client " << clientSocket << ": " << str << std::endl;
            str_buffer.erase(0, str_buffer.find("\r\n") + 2);

            for (std::list<Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
                if (it->get_fd() == clientSocket) {
                    if (it->registered_nick == false || it->registered_pass == false || it->registered_user == false){
                        authMessage(str, *it);
                    } else { 
                        regular_message(str, *it);
                    }
                    
                }
            }
            if (str_buffer.find("\r\n") == std::string::npos){
                return;
            }
        }
        std::string second_buffer(buffer);
        while(second_buffer.find("\n") != std::string::npos){
            std::string second_str = client_buffers[clientSocket]+ second_buffer.substr(0, second_buffer.find("\n"));
            client_buffers[clientSocket] = "";
            std::cout << "Received non standart message from client " << clientSocket << ": " << second_str << std::endl;
            second_buffer.erase(0, second_buffer.find("\n") + 1);

            for (std::list<Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
                if (it->get_fd() == clientSocket) {
                    it->write("The message you send is not standart, please use \\r\\n");
                    if (it->registered_nick == false || it->registered_pass == false || it->registered_user == false){
                        authMessage(second_str, *it);
                    } else { 
                        regular_message(second_str, *it);
                    }
                    
                }
            }
            if (second_buffer.find("\n") == std::string::npos){
                return;}
        }
        std::string receivedMessage(buffer);
        client_buffers[clientSocket] += receivedMessage;
    }

}

void Server::authMessage(std::string str, Client &client){
    Message msg;
    msg.Message_picker(str);
    if (msg.get_invalid() == true){
                std::cout << "Invalid message from client " << client.get_fd() << ": " << str << std::endl;
                //insert code here to send error message to client
                return;
    }
    std::cout << "Auth message from client " << client.get_fd() << ": " << msg.get_command() << std::endl;
    if (msg.get_command() == "PASS"){
        std::cout << "Password received from client: " << client.get_fd() << ": " << msg.get_params() << std::endl;
        if (msg.get_params() == this->password){
            std::cout << "Password accepted for client: " << client.get_fd() << std::endl;
            client.registered_pass = true;
        } else {
            std::string str(":local 464 * :Password incorrect. Disconnecting.");
            client.write(str);
            handleDisconnect(client.get_fd());
            return;//send error message
        }
    } else if (msg.get_command() == "NICK"){
        int nick_check = nick(msg.get_params(),client);
        if (nick_check == 2){
            return;
        }
        else if(nick_check == 1){
            handleDisconnect(client.get_fd());
            return;
        }
        client.registered_nick = true;
        std::cout << "Nickname "<< client.nickname<< " acepted for client: " << client.get_fd() << std::endl;
    } else if (msg.get_command() == "USER"){
        std::string str = msg.get_params();
        std::stringstream ss(str);

        if (!(ss >> client.username >> client.mode >> client.unused >> client.realname)) {
            // Extraction failed, handle the error (clear the stream, provide default values, etc.)
            
            std::cerr << "Error extracting values from the stringstream." << std::endl;
            std::string std(":local 461 " + client.nickname + " USER :Not enough parameters.");
            client.write(std);
            // Optionally, set default values for the fields
        } else {
            // Extraction was successful, proceed as before
            client.realname.erase(0, 1);
            std::cout << "Username " << client.username << " accepted for client: " << client.get_fd() << std::endl;
            client.registered_user = true;
        }
    }
    if (client.registered_nick == true && client.registered_pass == true && client.registered_user == true){
        client.hostname = "localhost";
        std::string reply = ":local 001 " + client.nickname + " :Welcome to the server " + client.nickname + "!" + client.username + "@" + client.hostname + " :Your real name is " + client.realname;
        client.write(reply);
        client.registered_full = true;
        //send welcome message
    }
}

void Server::regular_message(std::string full_msg, Client &client)
{
    std::cout << "Regular message from client " << client.get_fd() << ": " << full_msg << std::endl;
    Message message;
    message.Message_picker(full_msg);
	std::string msg = message.get_command();
    if (message.get_invalid() == true){
        //std::cout << "Invalid message from client " << client.get_fd() << ": " << message << std::endl;
        std::string str(":local 421 " + client.nickname + " " + msg + " :Unknown command");
        client.write(str);
        return;
    }
	if (msg.compare("JOIN") == 0)
		join(message.get_params(), client);//join(message.get_params(), *client);
    else if(msg.compare("NAMES") == 0)
        names(message.get_params(), client);
	else if (msg.compare("LIST") == 0)
		;//list(message.get_params(), *client);
	else if (msg.compare("KICK") == 0)
		kick(message.get_params(), client);
	else if (msg.compare("INVITE") == 0)
		;//invite(message.get_params(), *client);
	else if (msg.compare("MODE") == 0)
		mode(message.get_params(), client);
	else if (msg.compare("TOPIC") == 0)
		;//topic(message.get_params(), *client);
	else if (msg.compare("NICK") == 0)
		nick(message.get_params(), client);
	else if (msg.compare("QUIT") == 0)
		;//quit(message.get_params(), *client);
	else if (msg.compare("PRIVMSG") == 0)
		;//privmsg(message.get_params(), *client);
    else if (msg.compare("WHO") == 0)
        who(message.get_params(), client);
    else if (msg.compare("PART") == 0)
        ;//part(message.get_params(), *client
	else{
		std::string str(":local 421 " + client.nickname + " " + msg + " :Unknown command");
        client.write(str);
    }
}



void Server::newClient(int clientSocket) {
    // Check if the client socket has already been added
    if (addedSockets.find(clientSocket) != addedSockets.end()) {
        std::cout << "Client socket already added to epoll: " << clientSocket << std::endl;
        return;
    }

    fcntl(clientSocket, F_SETFL, fcntl(clientSocket, F_GETFL) | O_NONBLOCK);

    Client newClient(clientSocket);
    clients.push_back(newClient);
    

    struct epoll_event event;
    event.data.fd = clientSocket;
    event.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, clientSocket, &event) == -1) {
        perror("epoll_ctl");
        return;
    }

    addedSockets.insert(clientSocket);  // Mark the socket as added

    std::cout << "New client connected: " << clientSocket << std::endl;
}

void Server::handleDisconnect(int clientSocket) {

    // Remove client from epoll
    if (epoll_ctl(epollfd, EPOLL_CTL_DEL, clientSocket, NULL) == -1) {
        perror("epoll_ctl (delete)");
    }

    // Find and mark the client as disconnected
    for (std::list<Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (it->get_fd() == clientSocket) {
            int fd = it->get_fd();
            client_buffers[clientSocket] = "";
            //it->setDisconnected(true);
            std::list<Channel>::iterator cit;

            for (cit = this->channels.begin(); cit != this->channels.end(); ++cit) {
                if (cit->verifyUserInChannel(fd) == true) {
                    cit->removeClient(*it);
                    for (std::list<Client>::iterator it2 = clients.begin(); it2 != clients.end(); ++it2) {
                        if (cit->verifyUserInChannel(it2->get_fd()) == true) {
                            std::cout << "Sending message to client " << it2->get_fd() << ": " << it->getPrefix() << " PART " << cit->channelName << std::endl;
                            std::string str = ":" + it->getPrefix() + " PART " + cit->channelName + " :Leaving/Disconnecting";
                            it2->write(str);
                        }
                    }
                }
                
            }
            it->setDisconnected(true);
            break;
        }
    }


    addedSockets.erase(clientSocket);  // Remove the socket from addedSockets
}

 int Server::nick(std::string cmd, Client &client)
{
	std::stringstream cmds(cmd);
	std::string move;
    std::list<Client>::iterator cit;

	cmds >> move;
	if (move.empty() == true)
	{
		std::string str(":your.server.name 432 * :Erroneus nickname. Disconnecting.");
        client.write(str);//std::cout << "No nickname provided!" << std::endl;
		return 1;
	}
    if (cmd.find(" ") != std::string::npos)
    {
        std::string str(":your.server.name 432 * :Erroneus nickname. Disconnecting.");
        client.write(str);
        return 1;
    }
	if (client.nickname.compare(move) == 0)
	{

        std::string str(":local 436 " + client.nickname + " :Nickname is already the one you're using.");
		//std::cout << "The nickname provided is already your current nickname!" << std::endl;
        client.write(str);
		return 3;
	}

	for(cit = clients.begin(); cit != clients.end(); ++cit) 
	{
        if (cit->nickname.compare(move) == 0)
		{
			//std::cout << "The nickname provided is already in use!" << std::endl;
            std::string str(":local 433 " + client.nickname + " " + move + " :Nickname is already in use.");
            client.write(str);
			return 2;
		}
	}
    if (move[0] == '#'){   
        std::string str(":local 432 " + client.nickname + " " + move + " :Erroneus nickname.");
        client.write(str);
        return 1;
    }
	client.nickname = move;
    if (client.registered_full == true)
    {
        std::string str(":local 001 " + client.nickname + " :Welcome to the server " + client.nickname + "!");
        client.write(str);
    }
	//std::cout << "Your new nickname is " + move << std::endl;
    return 0;
}

void Server::join(std::string params, Client &client){
    std::stringstream ss(params);
    std::string channel;
    std::string pass;
    bool has_pass = false;
    ss >> channel;
    if (ss >> pass){
        has_pass = true;
    }
    if (has_pass == false){
        pass = "";
    }
    channel =  "#" + channel;
    std::list<Channel>::iterator cit;
    for(cit = this->channels.begin(); cit != this->channels.end(); ++cit){
        if (cit->channelName.compare(channel) == 0)
        {
            if (cit->addClient(client, pass) == true){
                for (std::list<Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
                    if (cit->verifyUserInChannel(it->get_fd()) == true){
                        std::string str = ":" + client.getPrefix() + " JOIN " + channel;
                        it->write(str);
                    }   
                }
            }
            return;
        }
    }
    
    Channel new_channel(channel, client);
    this->channels.push_back(new_channel);
    //std::string str = ":local 329 " + client.nickname + " " + channel + " :Channel";
    //client.write(str);
    std::string str = ":local 331 " + client.nickname + " " + channel + " :No topic is set";
    client.write(str);
    str = ":" + client.getPrefix() + " JOIN " + channel;
    client.write(str);
}


void Server::who(std::string params, Client &client) {
    std::stringstream ss(params);
    std::string channel;
    std::string flags; // To store optional flags like "o"
    ss >> channel >> flags;

    std::list<Channel>::iterator cit;

    for (cit = this->channels.begin(); cit != this->channels.end(); ++cit) {
        if (cit->channelName.compare(channel) == 0) {
            std::list<Client>::iterator clt;

            for (clt = this->clients.begin(); clt != this->clients.end(); ++clt) {
                if (cit->verifyUserInChannel(clt->get_fd())) {
                    // Check for "o" flag
                    bool isOperator = flags.find("o") != std::string::npos;
                    bool userIsOperator = cit->verifyAdminPrivilege(clt->get_fd()); // Modify this based on your logic
                    if (isOperator && !userIsOperator) {
                        continue; // Skip non-operators if "o" flag is specified
                    }

                    std::string flagsStr = userIsOperator ? "@" : "+";
                    std::string hopcount = "0"; // You may want to calculate this value
                    std::string str = ":" + this->hostname + " 352 " + client.nickname + " " +
                                      channel + " " + clt->username + " " + clt->hostname + " " +
                                      this->hostname + " " + clt->nickname + " H" + flagsStr +
                                      " :" + hopcount + " " + clt->realname;

                    std::cout << str << std::endl;
                    client.write(str);
                }
            }

            break; // We found the channel, no need to keep looking
        }
    }

    if (cit != this->channels.end()) {
        std::string endOfListResponse = ":" + this->hostname + " 315 " + client.nickname + " " + channel + " :End of WHO list";
        client.write(endOfListResponse);
        std::cout << endOfListResponse << std::endl;
    } else {
        std::string noSuchChannelResponse = ":" + this->hostname + " 403 " + client.nickname + " " + channel + " :No such channel";
        client.write(noSuchChannelResponse);
        std::cout << noSuchChannelResponse << std::endl;
    }
}

void Server::mode(std::string cmd, Client &client)
{
    std::stringstream cmds(cmd);
    std::string move;
    std::list<Channel>::iterator    cit;
    std::string msg;
    std::string channelName;
    cmds >> move;
    std::cout << "MODE: " << move << std::endl;
    std::cout << "fullMODE: " << cmd << std::endl;
    if (move.empty())
    {
        msg = ":local 461 " + client.nickname + " MODE :Not enough parameters1";
        client.write(msg);
        return;
    }
    for (cit = channels.begin(); cit != channels.end(); cit++)
    {
        if (cit->channelName == move)
            break;
    }
    if (cit == channels.end())
    {
        msg = ":local 403 " + client.nickname + " " + move + " :No such channel";
        client.write(msg);
    }
    else if (!cit->verifyUserInChannel((&client)->sock_fd))
    {
        msg = ":local 442 "+ client.nickname +" "+ cit->channelName + " :You're not on that channel";
        client.write(msg);
        return;
    }
    else if (!cit->verifyAdminPrivilege((&client)->sock_fd))
    {
        msg = ":local 482 "+ client.nickname +" "+ cit->channelName + " :You're not channel operator";
        client.write(msg);
        return;
    }
    else
    {
        std::string more;
        cmds >> move;
        if (cmds.eof())
        {
            
            msg = ":local 324 " + client.nickname + " " + cit->channelName + " " + cit->returnModes();
            client.write(msg);
            return;
        }
        cmds >> more;
        if (!more.empty() && !(move.c_str()[1] == 'o' || move.c_str()[1] == 'l' || move.c_str()[1] == 'k'))
        {
            msg = ":local 461 " + client.nickname + " MODE :Too many parameters1";
            client.write(msg);
            return;
        }
        if (more.empty() && (move.c_str()[1] == 'o' || move.c_str()[1] == 'l' || move.c_str()[1] == 'k'))
        {
            msg = ":local 461 " + client.nickname + " MODE :Not enought parameters2";
            client.write(msg);
            return;
        }
        if (!more.empty() && (move.c_str()[1] == 'o' || move.c_str()[1] == 'l' || move.c_str()[1] == 'k'))
        {
            std::string param;
            cmds >> param;
            if (!param.empty() && move.c_str()[1] == 'k')
            {
                msg = ":local 475 " + client.nickname + "  "+ cit->channelName + " :Invalid channel key";
                client.write(msg);
                return;
            }
            if (!param.empty())
            {
                msg = ":local 461 " + client.nickname + " MODE :Too many parameters2";
                client.write(msg);
                return;
            }
        }
        //int i = 1;
        int mode = -1;
        if (move.c_str()[0] == '+')
                mode = 0;
        else if (move.c_str()[0] == '-')
                mode = 1;
        else
        {
            msg = ":local 461 " + client.nickname + " MODE :Missing flags, not enought parameters";
            client.write(msg);
            return;
        }
        if (move.c_str()[1] == 'o' && mode == 0)
        {
            if(cit->verifyUserInChannel(getNickFD(more)))
            {
                cit->addAdminPrivilege(getNickFD(more), client.sock_fd);
                msg = ":" + client.getPrefix() + " MODE " + cit->channelName + " +o " + more;
                client.write(msg);
                notifyAllClientsInChannel(cit->channelName, "MODE " + cit->channelName + " +o " + more, client);
            }
            else
            {
                msg = ":local 442 "+ client.nickname +" "+ cit->channelName + " :You're not on that channel";
                client.write(msg);
            }
            return;
        }
        else if (move.c_str()[1] == 'o' && mode == 1)
        {
            if(cit->verifyUserInChannel(getNickFD(more)))
            {
                cit->removeAdminPrivilege(getNickFD(more), client.sock_fd);
                msg = ":" + client.getPrefix() + " MODE " + cit->channelName + " -o " + more;
                client.write(msg);
                notifyAllClientsInChannel(cit->channelName, "MODE " + cit->channelName + " -o " + more, client);
            }
            else
            {
                msg = ":local 442 "+ client.nickname +" "+ cit->channelName + " :You're not on that channel";
                client.write(msg);
            }
            return;
        }
        else if (move.c_str()[1] == 'i' && mode == 0)
        {
            if(cit->changeInviteOnly(true, client.sock_fd))
            {
                msg = ":" + client.getPrefix() + " MODE " + cit->channelName + " +i";
                client.write(msg);
                notifyAllClientsInChannel(cit->channelName, "MODE " + cit->channelName + " +i ", client);
            }
            else
            {
                msg = ":local 442 "+ client.nickname +" "+ cit->channelName + " :Channel is mode +i";
                client.write(msg);
            }
            return;
        }
        else if (move.c_str()[1] == 'i' && mode == 1)
        {
            if(cit->changeInviteOnly(false, client.sock_fd))
            {
                msg = ":" + client.getPrefix() + " MODE " + cit->channelName + " -i";
                client.write(msg);
                notifyAllClientsInChannel(cit->channelName, "MODE " + cit->channelName + " -i", client);
            }
            else
            {
                msg = ":local 442 "+ client.nickname +" "+ cit->channelName + " :Channel is mode -i";
                client.write(msg);
            }
            return;
        }
        else if (move.c_str()[1] == 't' && mode == 0)
        {
            if(cit->changeAOT(true, client.sock_fd))
            {
                msg = ":" + client.getPrefix() + " MODE " + cit->channelName + " +t";
                client.write(msg);
                notifyAllClientsInChannel(cit->channelName, "MODE " + cit->channelName + " +t ", client);
            }
            else
            {
                msg = ":local 472 "+ client.nickname +" "+ cit->channelName + " :Channel is mode +t";
                client.write(msg);
            }
            return;
        }
        else if (move.c_str()[1] == 't' && mode == 1)
        {
            if(cit->changeAOT(false, client.sock_fd))
            {
                msg = ":" + client.getPrefix() + " MODE " + cit->channelName + " -t";
                client.write(msg);
                notifyAllClientsInChannel(cit->channelName, "MODE " + cit->channelName + " -t ", client);
            }
            else
            {
                msg = ":local 472 "+ client.nickname +" "+ cit->channelName + " :Channel is mode -t";
                client.write(msg);
            }
            return;
        }
        else if (move.c_str()[1] == 'k' && mode == 0)
        {
            cit->setPassword(more, client.sock_fd);
            msg = ":" + client.getPrefix() + " MODE " + cit->channelName + " +k " + more;
            client.write(msg);
            notifyAllClientsInChannel(cit->channelName, "MODE " + cit->channelName + " +k " + more, client);
            return;
        }
        else if (move.c_str()[1] == 'k' && mode == 1)
        {
            cit->setPassword(NULL, client.sock_fd);
            msg = ":" + client.getPrefix() + " MODE " + cit->channelName + " -k";
            client.write(msg);
            notifyAllClientsInChannel(cit->channelName, "MODE " + cit->channelName + " -k ", client);
            return;  
        }
        else if (move.c_str()[1] == 'l' && mode == 1)
        {
            cit->setUsersLimit(-1, client.sock_fd);
            msg = ":" + client.getPrefix() + " MODE " + cit->channelName + " -l " + more;
            client.write(msg);
            notifyAllClientsInChannel(cit->channelName, "MODE " + cit->channelName + " -l " + more, client);
            return;
        }
        else if (move.c_str()[1] == 'l' && mode == 0)
        {
            if (std::atoi(more.c_str()) < 1)
            {
                msg = ":local 467 "+ client.nickname +" "+ cit->channelName + " :Key set with an invalid number of users";
                client.write(msg);
            }
            else
            {
                cit->setUsersLimit(std::atoi(more.c_str()), client.sock_fd);
                msg = ":" + client.getPrefix() + " MODE " + cit->channelName + " +l " + more;
                client.write(msg);
                notifyAllClientsInChannel(cit->channelName, "MODE " + cit->channelName + " +l " + more, client);
            }
            return;
        }
        msg = ":local 461 " + client.nickname + " MODE :Missing flag, not enought parameters";
        client.write(msg);
        return;
    }
}

void Server::notifyAllClientsInChannel(std::string channelName, std::string message, Client &client) {
    std::list<Channel>::iterator cit;

    for (cit = this->channels.begin(); cit != this->channels.end(); ++cit) {
        if (cit->channelName.compare(channelName) == 0) {
            std::list<Client>::iterator clt;

            for (clt = this->clients.begin(); clt != this->clients.end(); ++clt) {
                if (cit->verifyUserInChannel(clt->get_fd())) {
                    std::string str = ":" + client.getPrefix() +" " + message;
                    clt->write(str);
                }
            }

            break; // We found the channel, no need to keep looking
        }
    }
}


std::string Server::getClientNick(int clientfd){
    std::list<Client>::iterator cit;
    
    for (cit = clients.begin(); cit != clients.end(); cit++)
    {
        if (cit->sock_fd == clientfd)
            return cit->nickname;
    }
    return NULL;

}


Client Server::getClient(int clientfd)
{
    std::list<Client>::iterator cit;
    
    for (cit = clients.begin(); cit != clients.end(); cit++)
    {
        if (cit->sock_fd == clientfd)
            return *cit;
    }
    
    return *cit;
}

int Server::getNickFD(std::string user)
{
    std::list<Client>::iterator cit;
    
    for (cit = clients.begin(); cit != clients.end(); cit++)
    {
        if (cit->nickname == user)
            return cit->sock_fd;
    }
    return -1;
}

void Server::kick(std::string cmd, Client &client)
{
    std::stringstream cmds(cmd);
    std::string move;
    std::list<Channel>::iterator    cit;
    std::map<int,int>::iterator it;

    int userfd = 0;
    std::string msg;
    cmds >> move;
    if (move.empty())
    {
        msg = ":local 461 " + client.nickname + " KICK :Not enough parameters";
        client.write(msg);
        return;
    }
    for (cit = channels.begin(); cit != channels.end(); cit++)
    {
        if (cit->channelName == move)
            break;
    }
    if (cit == channels.end())
    {
        msg = ":local 403 " + client.nickname + " " + move + " :No such channel";
        client.write(msg);
    }
    else
    {
        cmds >> move;
        userfd = getUserFD(move);
        if (userfd == -1)
        {
            msg = ":local 401 " + client.nickname + " " + cit->channelName + " " + move + " :No such target";
            client.write(msg);
            return;
        }
        Client cclient = getClient(userfd);
        if (cit->kickClient(cclient, client))
        {
            msg = ":" + client.getPrefix() + " KICK " + cit->channelName + " " + cclient.nickname + " :";
            std::string reasons;
            while (!cmds.end)
            {
                cmds >> reasons;
                msg = msg + reasons;
            }
            cclient.write(msg);
            for (it = cit->channelUsers.begin(); it != cit->channelUsers.end(); it++)
            {
                cclient = getClient((*it).first);
                cclient.write(msg);
            }
        }
        return; 
    }
}

int Server::getUserFD(std::string user)
{
    std::list<Client>::iterator cit;
    
    for (cit = clients.begin(); cit != clients.end(); cit++)
    {
        if (cit->username == user)
            return cit->sock_fd;
    }
    return -1;
}


void Server::names(std::string cmd, Client &client)
{
    std::stringstream cmds(cmd);
    std::string move;
    std::list<Client>::iterator it;
    std::list<Channel>::iterator    cit;
    std::string msg = "";
    std::stringstream gettarget(cmd);
    std::string findtarget = "#";
    bool hastarget = 0;
    bool nick = 0;
    while (!gettarget.end || findtarget.c_str()[0] == '#')
        gettarget >> findtarget;
    if (!gettarget.end)
    {
        msg = ":local 461 " + client.nickname + " NAMES :Too many parameters";
        client.write(msg);
        return;
    }
    if (findtarget.c_str()[0] != '#')
        hastarget = 1;
    if (hastarget == 1)
    {
        if (getNickFD(findtarget) == -1 && getUserFD(findtarget) == -1)
        {
            msg = ":local 401 " + client.nickname + " " + findtarget + " :No such nick/channel";
            client.write(msg);
            return;
        }
        if (getUserFD(findtarget) != -1)
            nick = 1;
    }
    cmds >> move;
    if (move.empty())
    {
            msg = ":local 461 " + client.nickname + " NAMES :Not enough parameters";
            client.write(msg);
            return;
    }
    while(!cmds.end)
    {
        for (cit = channels.begin(); cit != channels.end(); cit++)
        {
            if (cit->channelName == move)
                break;
        }
        if (cit == channels.end())
        {
            msg = ":local 403 " + client.nickname + " " + move + " :No such channel";
            client.write(msg);
            return;
        }
        for (it = clients.begin(); it != clients.end(); it++)
        {
            if (cit->verifyUserInChannel((*it).sock_fd))
                msg += " " + (*it).nickname;             
        }
        if(hastarget == 0)
        {
            msg = ":local 353 " + client.nickname + " " + move;
            client.write(msg);
        }
        else
        {
            msg = ":local 353 " + findtarget + " " + move;
            if (nick == 0)
                getClient(getNickFD(findtarget)).write(msg);
            else
                getClient(getUserFD(findtarget)).write(msg);
        }
        cmds >> move;
        if (move == findtarget)
            break;
    }
    if(hastarget == 0)
    {
        msg = ":local 366 " + client.nickname + " :End of NAMES list";
        client.write(msg);
    }
    else
    {
        msg = ":local 366 " + findtarget + " :End of NAMES list";
        if (nick == 0)
            getClient(getNickFD(findtarget)).write(msg);
        else
            getClient(getUserFD(findtarget)).write(msg);
    }
}