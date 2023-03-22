#include "application.hpp"

#ifndef MATTDEAMONCLIENT
int  Application::create_directory_recursive(char *dir, mode_t mode)
{
    if (!dir) {
        errno = EINVAL;
        return 1;
    }

    if (strlen(dir) == 1 && dir[0] == '/') return 0;
    char *subdir = strdup(dir);
    create_directory_recursive(dirname(dir), mode);
    int ret = mkdir(subdir, mode);
    free(subdir);
    if (errno == EEXIST) return 0;
    return ret;
}

bool Application::CheckForInstance() {

    DIR* dir = opendir("/proc");
    if (dir == nullptr) {
        std::cerr << LIGHT_RED << "Failed to open /proc directory\n" << DEFAULT_COLOR;
        return (false);
    }
    pid_t pid = getpid();

    dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        // Check if the entry is a directory and its name is a number
        if (entry->d_type == DT_DIR && std::isdigit(entry->d_name[0])) {
            std::string cmdline_file = std::string("/proc/") + entry->d_name + "/status";
            std::ifstream ifs(cmdline_file);
            std::string cmdline;
            std::getline(ifs, cmdline);
            cmdline = cmdline.substr(cmdline.find("\t") + 1);

            // Check if the cmdline contains the process name
            if (cmdline.find("matt_daemon") != std::string::npos && std::stoi(entry->d_name) != pid) {
                std::string ret = "An instance of matt_daemon is already running pid: " + std::string(entry->d_name);
                std::cout << LIGHT_RED << ret << DEFAULT_COLOR << std::endl;
                reporter.error(ret);
                closedir(dir);
                return (true);
            }
        }
    }
    closedir(dir);
    return (false);
}

bool Application::CheckForLockFile() {

   if (CheckForInstance()) return true;
    if (FILE *file = fopen(LOCKFILE, "r+")) {
        fclose(file);
        std::string ret = "matt_daemon don't quit proprely, a lock file prevents starting: " + std::string(LOCKFILE) + " please remove it and try again";
        std::cout << LIGHT_RED << ret << DEFAULT_COLOR << std::endl;
        reporter.error(ret);
        return true;
    }

    std::string path = LOCKFILE;
    std::string directory = path.substr(0, path.find_last_of('/'));
    if (create_directory_recursive((char*)directory.c_str(), S_IRWXU | S_IRWXO))
        std::cout << "can't log to file:" << path;

    std::ofstream outfile (LOCKFILE);
    outfile.close();
    return false;
}

void Application::configureLogger() {
    std::string path = LOGGING_DEFAULT_PATH;
    if (path.find('/') != std::string::npos) {
        std::string directory = path.substr(0, path.find_last_of('/'));
        if (create_directory_recursive((char*)directory.c_str(), S_IRWXU | S_IRWXO))
            std::cout << LIGHT_RED << "can't log to file: " << WHITE_BOLD << path << DEFAULT_COLOR << std::endl;
    }
    reporter.init(LOGGING_DEFAULT_PATH);
}
#endif

void Application::start() {
    #ifdef MATTDEAMONCLIENT
        client.start(preference.ip, preference.port, preference.master_password);
    #else
        reporter.system("start");

        if (CheckForLockFile()) {
            reporter.system("stop");
            reporter.close();
            return ;
        }

        std::string ret = server.configure(preference.port, preference.master_password);
        if (ret.size()) {
            std::cout << LIGHT_RED << ret << std::endl;
            remove(LOCKFILE);
            reporter.system("stop");
            reporter.close();
            return ;
        }

        server.start();
        remove(LOCKFILE);
        reporter.system("stop");
        reporter.close();
    #endif
}

void Application::stop() {
    #ifdef MATTDEAMONCLIENT
        client.clear();
    #else
        server.stop();
    #endif
}

void Application::initWithArg(int ac, char **av) {
    preference.configure(ac, av);

    #ifdef MATTDEAMONCLIENT
    client.encrypter = ([&] (std::string message) {
        std::string encryptedMessage;
        encryptedMessage = cryptage.crypter(message, preference.crytage_key);
        return encryptedMessage;
    });

    client.decrypter = ([&] (std::string message) {
        std::string decryptedMessage;
        decryptedMessage = cryptage.decrypter(message, preference.crytage_key);
        return decryptedMessage;
    });
    #else
        if (geteuid() != 0) {
            fprintf(stderr, "%sNeed to be run as Root\n", LIGHT_RED);
            exit(1);
        }

        configureLogger();
        server.decrypter = ([&] (std::string message) {
            std::string decryptedMessage;
            decryptedMessage = cryptage.decrypter(message, preference.crytage_key);
            return decryptedMessage;
        });

        server.encrypter = ([&] (std::string message) {
            std::string encryptedMessage;
            encryptedMessage = cryptage.crypter(message, preference.crytage_key);
            return encryptedMessage;
        });


        server.onMessageReceive = ([&] (const char *command) {
            std::string ret;
            if (!strcmp(command, "quit")) {
                server.stop();
            }
            return ret;
        });

    #endif

}

void Application::sigint() {
    #ifdef MATTDEAMONCLIENT
        client.clear();
    #else
        server.stop();
     #endif
}

void Application::sigwinch() {
    #ifdef MATTDEAMONCLIENT
        client.redraw();
    #endif
}
