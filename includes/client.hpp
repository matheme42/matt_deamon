#ifndef CLIENT_HPP
#define CLIENT_HPP

// socket
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>

// std
#include <vector>
#include <functional>
#include <string>
#include <string.h>
#include <algorithm>

// termcaps
#include <termios.h>
#include <termcaps.hpp>
#include <color.hpp>

#include <logging.hpp>

class Client
{
    #define DEFAULT_PROMPT BLUE << "matt_deamon " << DARK_BLUE << "✗ " << DEFAULT_COLOR
    #define DEFAULT_PROMPT_SIZE 12

    std::vector <std::string> commandList = {
    "quit"};

    private:
        Termcaps termcaps;
        std::string line;
        int cursor_position;
        int autocompletionStringIdx;
        std::string autocompletionString;
        struct termios original_termios, new_termios;

        std::vector <std::string> history;
        int history_index;
        bool waitingForRemote;
        bool authenticated;
        std::string master_password;

        int client_fd = 0;
        char buffer[8192] = { 0 };


        int manageControlKey(int input);
        int managekey(int input);
        void autocompletion();
        void addLineTohistory();
        void restoreKeyboard();
        void configureKeyboard();

        void sendCommandLineInRemote();
        void readCommandLineInRemote();
        
        void tryAutoConnect();
        bool listen;

    public:
        Client() = default;
        Client(const Client &client) = default;
        ~Client() = default;
            
        Client &operator=(const Client &client) = default;

        std::function<std::string(std::string)> encrypter;
        std::function<std::string(std::string)> decrypter;

        void start(std::string host, int port, std::string master_password);
        void start();
        void stop();
        void clear();
        void redraw();
        void newPrompt();
};




#endif