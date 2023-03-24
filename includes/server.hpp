#ifndef SEVER_HPP
#define SEVER_HPP

#include <color.hpp>

#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <functional>
#include <string>
#include <fstream>
#include <sys/stat.h>
#include <algorithm>

#include <logging.hpp>

class Server {
    #define MAX_CLIENT 3

    private:
        bool                running;
        int                 server_fd;
        std::vector<int>    client_socket;
        std::vector<int>    authenticate_client;
        char                buffer[1024];
        struct sockaddr_in  address;
        socklen_t           addrlen;
        std::string         master_password;


        std::vector <std::string> taskNameList;
        int Demonize();
    public:
        Server();
        Server(const Server &server) = default;
        ~Server() = default;
        
        Server &operator=(const Server &server) = default;

        std::function<std::string(const char *)> onMessageReceive;
        std::function<std::string(std::string)> decrypter;
        std::function<std::string(std::string)> encrypter;

        std::string configure(int port, std::string master_password);
        void start();
        void listenForConnection();
        void checkSocketStatus(int socket);
        void checkSocketMessage(int socket);
        std::string readClientSocket(int socket);
        void stop();
        void sendMessage(int socket, std::string message);

};


#endif