#include "server.hpp"

void Server::stop() {
    running = false;
}

Server::Server() {
    bzero(buffer, sizeof(buffer));
    server_fd = 0;
}

int Server::Demonize() {
    pid_t pid, sid;
    /* données qui peut recevoir l'identifiant d'un processus
    (ce que l'on appelle le pid: process identifier) et qui sont
    par définition des entiers */
 
    pid = fork();
    /* création d'un processus appelé processus fils qui fonctionne
    en parallèle du premier processus appelé processus père */
 
    if( pid < 0)
        return ( EXIT_FAILURE );
    else if( pid > 0) {
        reporter.system("daemonize pid: " + std::to_string(pid));
        exit ( EXIT_SUCCESS );
    }
    /* fork ne renvoit pas la même chose au deux processus.
    > pour le fils il renvois 0
    > pour le père il renvois le pid du fils dans mon cas on arrête le
    programme pour le père */
 
    umask(0);
    /* donne les droit par défaut 0777 */
 
    sid = setsid();
    /* setsid obtient pour valeur le pid du processus fils mais échoue
    dans certains cas comme par exemple celui ou le processus fils à le
    même pid qu'un processus déjà existant */
 
    if( sid < 0 )
    {
        perror( "daemonize::sid" );
        return ( EXIT_FAILURE );
    }
    /* en cas d'échec de setsid on a sid < 0 et alors on interrompt la
    procedure */
 
   // if( chdir("/") < 0 )
  //  {
   //     perror( "daemonize::chdir" );
   //     return ( EXIT_FAILURE );
   // }

    close( STDIN_FILENO );
    close( STDOUT_FILENO );
    close( STDERR_FILENO );
    /* le fils partage les descripteurs de fichier du père sauf si on les
    ferme et dans ce cas ceux du père ne seront pas fermés */


   if (open("/dev/null", O_RDWR) != 0)
   {
       return ( EXIT_FAILURE );
   }

   (void) dup(0);
   (void) dup(0);

    return ( EXIT_SUCCESS );
}

std::string Server::configure(int port, std::string master_password) {
    std::string ret;

    running = false;
    this->master_password = master_password;

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        ret = ret + "socket failed";
        ret = ret + strerror(errno);
        reporter.error(ret);
        return (ret);
    }

    int opt = 1;
    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        ret = ret + "setsockopt";
        ret = ret + strerror(errno);
        reporter.error(ret);
        return (ret);
    }
    reporter.system("socket created");

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

   // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        ret = "the port: " + std::to_string(port) + " is already use";
        reporter.error(ret);
        return (ret);
    }
    reporter.system("socket binded on port " + std::to_string(port));
 
    if (listen(server_fd, 3) < 0) {
        ret = ret + "listen";
        ret = ret + strerror(errno);
        reporter.error(ret);
        return (ret);
    }

    fcntl(server_fd, F_SETFL, O_NONBLOCK);
    running = true;
/*
    if(Demonize()) {
        reporter.error("can't convert into daemon");
        ret = ret + "can't convert into daemon";
        return (ret);
    }*/

    return (ret);
}

void Server::listenForConnection() {
    int socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    if (socket >= 0) {
        if (client_socket.size() == MAX_CLIENT) {
            reporter.system("someone try to connect but server is full");
            sendMessage(socket, "server is full");
            close(socket);
        } else {
            if (master_password.size() != 0) {
                reporter.system("someone try connect to the server");
                sendMessage(socket, "password: ");
            } else {
                reporter.system("someone connect to the server");
                authenticate_client.push_back(socket);
            }
            fcntl(socket, F_SETFL, O_NONBLOCK);
            client_socket.push_back(socket);
        }
    }
}

std::string Server::readClientSocket(int socket) {
    char        *line = NULL;
    int         c = 0;
    size_t      size = 0;
    std::string str;

    if (read(socket, &c, 1) == 0) {
        reporter.system("someone disconnect from the server");
        client_socket.erase(std::find(client_socket.begin(), client_socket.end(), socket));
        authenticate_client.erase( std::remove( authenticate_client.begin(), authenticate_client.end(), socket), authenticate_client.end());
        return "";
    }
    if (getline(&line, &size, fdopen(socket, "r")) < 0)  {
        return "";
    }

    str = str + (char*)&c + line;
    free(line);
    return str;
}

void Server::checkSocketMessage(int socket) {
    std::string command_ret;
    std::string request;
    int         ret;

    if (socket <= 0) return ;
    request = readClientSocket(socket);
    if (request.empty()) return ;

        /// check for authentication
    if (std::find(authenticate_client.begin(), authenticate_client.end(), socket) == authenticate_client.end()) {
        
        int cmp;
        if (decrypter) cmp = strcmp(decrypter(request).c_str(), master_password.c_str());
        else cmp = strcmp(request.c_str(), master_password.c_str());
        if (cmp != 0) {
            sendMessage(socket, "password: ");
        } else {
            reporter.system("successfull connecting to the server");
            authenticate_client.push_back(socket);
            sendMessage(socket, "authenticate");
        }
        return ;
    }

    if (onMessageReceive) {
        if (decrypter) command_ret = onMessageReceive(decrypter(request).c_str());
        else command_ret = onMessageReceive(request.c_str());
    }

    if (command_ret[0] != '\0') sendMessage(socket, command_ret);
    else sendMessage(socket, "\a");
}

void Server::start() {
    reporter.system("server start");
    while (running) {
        usleep(90000);
        listenForConnection();
        for (int i = 0; i < client_socket.size(); i++) checkSocketMessage(client_socket.at(i));
    }

    usleep(10000);
    while (client_socket.size()) {
        int socket = client_socket.at(0);
        client_socket.erase(client_socket.begin());
        sendMessage(socket, "exit");
        shutdown(socket, SHUT_RDWR);
        close(socket);
    }
    if (server_fd > 0) {
        shutdown(server_fd, SHUT_RDWR);
        close(server_fd);
    }
    reporter.system("server stop");
}

void Server::sendMessage(int socket, std::string message) {
    if (encrypter) {
        std::string encrypted = encrypter(message);
        send(socket, encrypted.c_str(), encrypted.size(), 0);
        return ;
    }
    send(socket, message.c_str(), message.size(), 0);
}
