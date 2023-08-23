#include "Server.hpp"


Server::Server(int port, const char *password)
{
	this->port = port;
	this->password = password;
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
        // Handle the received message here
        // For example, you can broadcast the message to other clients
        // ...
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
            std::string str(":your.server.name 464 * :Password incorrect. Disconnecting.");
            client.write(str);
            handleDisconnect(client.get_fd());
            return;//send error message
        }
    } else if (msg.get_command() == "NICK"){
        client.nickname = msg.get_params();
        std::cout << "Nickname accepted for client: " << client.get_fd() << std::endl;
        client.registered_nick = true;
    } else if (msg.get_command() == "USER"){
        client.username = msg.get_params();
        std::cout << "Username accepted for client: " << client.get_fd() << std::endl;
        client.registered_user = true;
    }
    if (client.registered_nick == true && client.registered_pass == true && client.registered_user == true){
        std::string reply = ":local 001 " + client.nickname + " :Welcome to the server!\r\n";
        //send(client.get_fd(), reply.c_str(), reply.length(), 0);
        client.write(reply);
        //send welcome message
    }
}

void Server::regular_message(std::string message, Client &client)
{
    Message msg;
    msg.Message_picker(message);
    std::cout << "Regular message from client " << client.get_fd() << ": " << msg.get_command() << std::endl;
    //stuff
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
            client_buffers[clientSocket] = "";
            it->setDisconnected(true);
            break;
        }
    }


    addedSockets.erase(clientSocket);  // Remove the socket from addedSockets
}

