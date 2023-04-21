#ifndef TINTIN_REPORTER_HPP
#define TINTIN_REPORTER_HPP

#include <string>
#include <ostream>
#include <iostream>
#include <fstream>

#include <ctime>
#include <iomanip>
#include <chrono>
#include <sstream>

#include <syslog.h>

#include <color.hpp>

class Tintin_reporter {
    #define LOGGING_DEFAULT_PATH "/var/log/matt_daemon/matt_daemon.log"

    private:
        std::string outputfile;

        std::string getTime();
        void log(std::string message, std::string color, std::string level);
    public:
        Tintin_reporter() = default;
        Tintin_reporter(const Tintin_reporter &reporter) = default;
        ~Tintin_reporter() = default;
        
        Tintin_reporter &operator=(const Tintin_reporter &reporter) = default;

        void init(std::string outputfile);
        void close();

        void system(std::string message);
        void prompt(std::string message);
        void command(std::string message);
        void signal(std::string message);
        void error(std::string message);
};

extern Tintin_reporter reporter;

#endif