#include "../../includes/Bot.hpp"

Bot::~Bot()
{
    
}

void Bot::bot_setPasword(std::string const& password)
{
    this->_password = password;
}

void Bot::bot_setPort(int port)
{
    this->_port = port;
}

void Bot::signalHandler(int signum)
{
    (void)signum;
    std::cout << RED << "Bot disconnected" << RESET << std::endl;
    system("leaks bot");
    exit(0);
}

void Bot::bot_createSocket()
{
    if ((_fd = socket(AF_INET, SOCK_STREAM, 0)) <= 0)
        throw std::runtime_error("socket() failed");
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(_port);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    if (::connect(_fd, (struct sockaddr *) &addr, sizeof(addr)) < 0)
        throw std::runtime_error("connect() failed");
    std::cout << BLUE << "Bot connected to server" << RESET << std::endl;
}

void Bot::bot_run()
{
    int login = 0;
    while (1)
    {
        if (!login)
        {
            std::string msg = "CAP BOT\nPASS " + _password +"\nNICK BOT\n";
            Utils::writeMessage(_fd, msg);
            login = 1;
        }
        char buffer[1024];
        int bytes_received = recv(_fd, buffer, sizeof(buffer), 0);
        buffer[bytes_received] = '\0';
        std::string tmp = buffer;
        if (bytes_received <= 0)
        {
            close(_fd);
            throw std::runtime_error("recv() failed");
        }
        std::stringstream ss(tmp);
        std::string ctr;
        ss >> ctr;
        if (ctr == "bot")
        {
            ss >> tmp;
            ss >> ctr;
            std::string msg = ctr;
            while (ss >> ctr)
                msg += " " + ctr;
            std::string writeCmd = "NOTICE * " + tmp + " " + msg + "\r\n";
            Utils::writeMessage(_fd, writeCmd);
        }
    }
}

void Bot::manageBot(int port, std::string const& password)
{
    bot_setPasword(password);
    bot_setPort(port);
    bot_createSocket();
    bot_run();
}
