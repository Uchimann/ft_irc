#pragma once

#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Utils.hpp"

class Bot
{
    public:
        // variables
        int _fd;
        int _port;
        std::string _password;
        // methods
        Bot(): _fd(0), _port(0), _password("") {};
        ~Bot();
        void bot_setPort(int);
        void bot_setPasword(std::string const&);
        void bot_createSocket();
        void bot_run();
        void manageBot(int, std::string const&);
        static void signalHandler(int);
};
