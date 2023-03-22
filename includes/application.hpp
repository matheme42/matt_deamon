#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <server.hpp>
#include <client.hpp>
#include <option.hpp>
#include <logging.hpp>
#include <cryptage.hpp>

#include <libgen.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <string>


class Application {
    #define LOCKFILE "/var/lock/matt_daemon.lock"

    private:
        /// modules
        Server          server;
        Client          client;
        Option          preference;
        Cryptage        cryptage;

        int  create_directory_recursive(char *dir, mode_t mode);
        void configureLogger();
        bool CheckForLockFile();
        bool CheckForInstance();
    public:
        Application();
        Application(const Application &app);
        ~Application();
            
        Application &operator=(const Application &app);
        /// initialize
        void initWithArg(int ac, char **av);

        /// start/stop
        void start();
        void stop();

        // reagir a signal
        void sigint();
        void sigwinch();
        void sighup();

};

#endif
