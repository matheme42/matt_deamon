#include "client.hpp"

void Client::stop() {
    listen = false;
}

void Client::clear() {
    if (!listen) return ;
    std::cout << '\n';
    newPrompt();
    fflush(stdout);
}

void Client::autocompletion() {

    int localAutocompletionStringIdx = 0;

   // if (line.size() == 0) return ;
    for (size_t i = 0; i < commandList.size(); i++) {
        if (commandList[i].find(line) == 0) {
            localAutocompletionStringIdx++;
            if (localAutocompletionStringIdx <= autocompletionStringIdx) {
                continue ;
            }
            autocompletionStringIdx = localAutocompletionStringIdx;
            auto autoComplete = commandList[i].substr(line.size());
            if (autocompletionString.size() > 0) {
                int del = autocompletionString.size();
                for (int i = 0; i < del; i++) {
                    termcaps.del();
                }
            }

            autocompletionString = autoComplete;
            std::cout << autoComplete;
            return ;
        }
    }
    if (localAutocompletionStringIdx != 0) {
        autocompletionStringIdx = 0;
        autocompletion();
    }
}


int Client::manageControlKey(int input) {
    if (input == 4) { //ctrl-d
        if (line.size() == 0) {
            termcaps.delLine();
            newPrompt();
            std::cout << '\n';
            stop();
        } 
    } else if (input != 9 && autocompletionString.size() > 0) {
        line = line + autocompletionString;
        autocompletionStringIdx = 0;
        cursor_position += autocompletionString.size();
        autocompletionString.clear();
    }
    if (input == 27) { /// arrow key
    input = getchar();
    if (input == 91) {
        input = getchar();
        if (input == 49) {  /// ctrl + arrow
            getchar();getchar();getchar();
        }
        else if (input == 65) {
            if (history.size() > 0) {
                if (history_index > 0) {
                        termcaps.delLine();
                        line.clear();
                        newPrompt();
                        line = history[--history_index];
                        cursor_position = line.size();
                        std::cout << line;
                    } else {
                        history_index = -1;
                        termcaps.delLine();
                        newPrompt();
                    }
                }
            } else if (input == 66) {
            if (history.size() > 0) {
                history_index++;
                if (history_index >= (int)history.size()) {
                    termcaps.delLine();
                    line.clear();
                    history_index = history.size();
                    cursor_position = 0;
                    newPrompt();
                } else {
                    termcaps.delLine();
                    line.clear();
                    newPrompt();
                    line = history[history_index];
                    cursor_position = line.size();
                    std::cout << line;
                }
            }
        } else if (input == 67 && cursor_position < (int)line.size()) {
            cursor_position += 1;
            if ((cursor_position + DEFAULT_PROMPT_SIZE + 1) % termcaps.column_count == 0) {
                termcaps.moveDown();
                return (1);
            }
            termcaps.moveRight();
        } else if (input == 68 && cursor_position > 0) {
            if ((cursor_position + DEFAULT_PROMPT_SIZE + 1) % termcaps.column_count == 0) {
                cursor_position -= 1;
                termcaps.moveUp();
                termcaps.moveRight(termcaps.column_count);
                return (1);
            }
            cursor_position -= 1;
            termcaps.moveLeft();
        }
        return (1);
    }
    } else if (input == 127) { /// backspace
        if (cursor_position > 0) {
            if ((cursor_position + DEFAULT_PROMPT_SIZE + 1) % termcaps.column_count == 0) {
                termcaps.delUp();
            } else {
                termcaps.del();
            }
            cursor_position -= 1;
            line.erase(cursor_position, 1);
            termcaps.saveCursorPosition();
            std::cout << line.substr(cursor_position, line.size() - cursor_position);
            std::cout << ' ';
            termcaps.restoreCursorPosition();
        }
        return (1);
    } else if (input == -1) {
        return (1);
    } else if (input == 9) { /// tabulation
        autocompletion();
        return (1);
    } else if (input == 12) { /// clear
        termcaps.clear();
        newPrompt();
        return (1);
    }
    return (0);
}

int Client::managekey(int input) {
    std::cout << (char)input;
    if (input == '\n') return (1);
    cursor_position += 1;
    if (cursor_position > (int)line.size())
        line.push_back((char)input);
    else {
        termcaps.saveCursorPosition();
        line.insert(cursor_position - 1, 1, (char)input);
        std::cout << line.substr(cursor_position, line.size() - cursor_position);
        termcaps.restoreCursorPosition();
    }
    if ((cursor_position + DEFAULT_PROMPT_SIZE + 1) % termcaps.column_count == 0) {
        termcaps.moveDown();
    }
    return (0);
}

void Client::addLineTohistory() {
    if (line.size() == 0 || !authenticated) return ;
    auto it = std::find(history.begin(), history.end(), line);
    if (it != history.end()) history.erase(it);
    history.push_back(line);
    history_index = history.size();
}

void Client::newPrompt() {
    line.clear();
    cursor_position = 0;
    termcaps.delLine();
    if (!authenticated && master_password.size() == 0) {
        std::cout << DARK_BLUE << "requesting password: " << DEFAULT_COLOR;
        return ;
    }
    std::cout << DEFAULT_PROMPT;
}

void Client::configureKeyboard() {
    // Sauvegarde les paramètres du terminal d'origine
    tcgetattr(STDIN_FILENO, &original_termios);
   // Configure le nouveau terminal pour l'entrée clavier non bloquante
    new_termios = original_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
    fcntl(0, F_SETFL, O_NONBLOCK);
}

void Client::restoreKeyboard() {
    // Restaure les paramètres du terminal d'origine
    tcsetattr(STDIN_FILENO, TCSANOW, &original_termios);
}

void Client::sendCommandLineInRemote() {
    if (encrypter) {
        std::string encrypted = encrypter(line);
        send(client_fd, encrypted.c_str(), encrypted.size() + 1, 0);
    } else {
        send(client_fd, line.c_str(), line.size() + 1, 0);
    }
    waitingForRemote = true;
    addLineTohistory();
}

void Client::tryAutoConnect() {
    if (master_password.size() == 0) return ;
    if (encrypter) {
        std::string encrypted = encrypter(master_password);
        send(client_fd, encrypted.c_str(), encrypted.size() + 1, 0);
    } else {
        send(client_fd, master_password.c_str(), master_password.size() + 1, 0);
    }
    waitingForRemote = true;
    master_password.clear();
}

void Client::readCommandLineInRemote() {
    int valread = read(client_fd, buffer, 8192);
    if (valread <= 0) return ;
    if (decrypter) {
        std::string decrypted = decrypter(buffer);
        strcpy(buffer, decrypted.c_str());
    }

    if (!strcmp(buffer, "exit")) {
        if (!waitingForRemote) std::cout << '\r' << DEFAULT_PROMPT;
        if (!waitingForRemote) std::cout << LIGHT_RED << "Connection closed by remote host" << DEFAULT_COLOR << std::endl;
        fflush(stdout);
        stop();
    } else if (!strcmp(buffer, "server is full")) {
        std::cout << '\r' << DEFAULT_PROMPT << LIGHT_RED << buffer << '\n' << DEFAULT_COLOR;
        fflush(stdout);
        stop();
    }
    else if (!strcmp(buffer, "password: ")) {
        authenticated = false;
    } else if (!strcmp(buffer, "authenticate")) {
        authenticated = true;
        termcaps.delLine();
    } else if (buffer[0] != '\0' && buffer[0] != '\n') {
        std::cout << buffer;
        if (valread != 8192 && buffer[8191] == '\0') std::cout << std::endl;
    }
    waitingForRemote = false;

    if (listen && valread != 8192 && buffer[8191] == '\0') newPrompt();
    bzero(buffer, valread);
    if (!authenticated && master_password.size() > 0) tryAutoConnect();
}

void Client::start() {
    std::string command_ret;
    int key;

    history_index = 0;
    cursor_position = 0;
    authenticated = true;
    autocompletionStringIdx = 0;
    listen = true;
    waitingForRemote = false;

    newPrompt();
    configureKeyboard();
    while (listen) {
        usleep(9000);
        readCommandLineInRemote();
        if ((key = getchar()) < 0 || waitingForRemote || manageControlKey(key) || !managekey(key)) continue;
        if (line.size() != 0) sendCommandLineInRemote();
        newPrompt();
    }
    restoreKeyboard();
}

void Client::start(std::string host, int port, std::string master_password) {
    bzero(buffer, sizeof(buffer));

    int status;
    struct sockaddr_in serv_addr;
    this->master_password = master_password;
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        std::cout << LIGHT_RED << "Socket creation error\n" << DEFAULT_COLOR;
        return ;
    }
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
  
    if (host.size() == 0) host = "127.0.0.1";

    // Convert IPv4 and IPv6 addresses from text to binary
    if (inet_pton(AF_INET, host.c_str(), &serv_addr.sin_addr) <= 0) {
        std::cout << LIGHT_RED << "Invalid address/ Address not supported\n" << DEFAULT_COLOR;
        return ;
    }

    struct timeval timeout;
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;
    if (setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
        std::cout << LIGHT_RED << "Failed to set SO_RCVTIMEO: " << strerror(errno) << DEFAULT_COLOR << std::endl;
        return ;
    }

    if (setsockopt(client_fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
        std::cout << LIGHT_RED << "Failed to set SO_SNDTIMEO: " << strerror(errno) << DEFAULT_COLOR << std::endl;
        return ;
    }
  
    if ((status = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)))
        < 0) {

        std::cout << LIGHT_RED << "Connection Failed\n" << DEFAULT_COLOR;
        return ;
    }

    std::cout << LIGHT_GREEN << "Connected to " << host << ":" << port << DEFAULT_COLOR << std::endl;
    fflush(stdout);

    // set socket to non blocking
    fcntl(client_fd, F_SETFL, O_NONBLOCK);

    // start listen stdin
    start();
  
    // closing the connected socket
    close(client_fd);
}

void Client::redraw() {
    termcaps.clear();
    std::cout << DEFAULT_PROMPT << line;
    if (autocompletionString.size() > 0) {
        std::cout << autocompletionString;
    }
    fflush(stdout);
}