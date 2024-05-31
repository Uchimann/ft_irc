#include "../../includes/Bot.hpp"
#include <signal.h>

int main(int ac, char **av)
{
    Bot newbot;
    try {
        if (ac != 3)
            throw std::runtime_error("./bot <port> <password>");
        if (!Utils::portIsValid(av[1]))
            throw std::runtime_error("invalid port");
        signal(SIGINT, Bot::signalHandler);
        newbot.manageBot(atoi(av[1]), av[2]);
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}
