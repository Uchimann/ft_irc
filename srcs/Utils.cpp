#include "../includes/Utils.hpp"

void Utils::writeMessage(int socket, std::string const& message)
{
    if ((write(socket, message.c_str(), message.length())) < 0)
        std::cout << "Message cannot send!" << std::endl;
}

void Utils::writeAllMessage(std::vector<int> const& fds, std::string const& message)
{
    for (std::vector<int>::const_iterator it = fds.begin(); it != fds.end(); ++it) {
        writeMessage(*it, message);
    }
}

std::string Utils::intToString(int n)
{
    std::stringstream ss;
    ss << n;
    std::string str = ss.str();
    return str;
}


int Utils::portIsValid(std::string const& port)
{

    for (size_t i = 0; i < port.size(); ++i) 
    {
        if (!isdigit(port[i]))
            return 0;
    }

    if (atoi(port.c_str()) < 1024 || atoi(port.c_str()) > 49151)
        return 0;
    return 1;
}

std::string Utils::welcome()
{
      std::string ascii_art = 
        "  ____        _        _____                            _   \n"
        " |  _ \\      | |      / ____|                          | |  \n"
        " | |_) |_   _| |_ ___| |     ___  _ __  _ __   ___  ___| |_ \n"
        " |  _ <| | | | __/ _ \\ |    / _ \\| '_ \\| '_ \\ / _ \\/ __| __|\n"
        " | |_) | |_| | ||  __/ |___| (_) | | | | | | |  __/ (__| |_ \n"
        " |____/ \\__, |\\__\\___|\\_____|\\___/|_| |_|_| |_|\\___|\\___|\\__|\n"
        "         __/ |                                              \n"
        "   _____|___/     ______                                   \n"
        "  / ____|  /\\    |___  /                                   \n"
        " | (___   /  \\      / /                                    \n"
        "  \\___ \\ / /\\ \\    / /                                     \n"
        "  ____) / ____ \\  / /__                                    \n"
        " |_____/_/    \\_\\/_____|                                   \n";
                           
    return ascii_art;
}
