#include "../../includes/Server.hpp"

std::string infoMessage()
{
    std::string message;

    message += " +--------------------------------------------------------+\n";
    message += "| * Current commands:                                      |\n";
    message += "|   - BOT                                                  |\n";
    message += "|   - CAP                                                  |\n";
    message += "|   - HELPME                                               |\n";
    message += "|   - INFO                                                 |\n";
    message += "|   - INVITE                                               |\n";
    message += "|   - JOIN                                                 |\n";
    message += "|   - KICK                                                 |\n";
    message += "|   - LIST                                                 |\n";
    message += "|   - MODE                                                 |\n";
    message += "|   - NICK                                                 |\n";
    message += "|   - NOTICE                                               |\n";
    message += "|   - OPER                                                 |\n";
    message += "|   - PART                                                 |\n";
    message += "|   - PASS                                                 |\n";
    message += "|   - PRIVMSG                                              |\n";
    message += "|   - QUIT                                                 |\n";
    message += "|   - TOPIC                                                |\n";
    message += "|   - USER                                                 |\n";
    message += "|   - WHO                                                  |\n";
    message += "|   - WHOIS                                                |\n";
    message += " +--------------------------------------------------------+\n";
    return (message);
}

void Server::Info(std::vector<std::string>& params, Client &cli)
{
    passChecker(cli);
    (void)params;
    Utils::writeMessage(cli._cliFd, RPL_INFO(cli._nick, infoMessage()));
}
