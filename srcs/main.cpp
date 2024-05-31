#include "../includes/Server.hpp"
#include <signal.h>
int main(int ac, char **av)
{
    try 
    {
        if (ac != 3)
            throw std::runtime_error("Input Must Be ./ircserv <port> <password>");
        if (!Utils::portIsValid(av[1]))
            throw std::runtime_error("Error! Invalid port");
        signal(SIGINT,Server::signalHandler);
        signal(SIGQUIT,Server::signalHandler);
        Server::getInstance()->manageServer(atoi(av[1]), av[2]); 
    } 
    catch (std::exception& e) 
    {
        std::cout << e.what() << std::endl;
    }
    Server::getInstance()->clear_server();
    //system("leaks ircserv");
    return 0;
}
