#include "../../includes/Server.hpp"
#include <sstream> // stringstream için gerekli

void Server::Part(std::vector<std::string>& params, Client& cli)
{
    passChecker(cli);

    if (params.size() < 1) {
        Utils::writeMessage(cli._cliFd, ERR_NEEDMOREPARAMS(cli._nick, "PART"));
        return;
    }

    std::vector<std::string> channels;
    std::string currentChannel;
    std::string paramStr = params[0];

    // Kanal isimlerini parçalara ayırma
    for (size_t i = 0; i < paramStr.length(); ++i) {
        if (paramStr[i] == ',' && !currentChannel.empty()) {
            channels.push_back(currentChannel);
            currentChannel.clear();
        } else {
            currentChannel += paramStr[i];
        }
    }
    if (!currentChannel.empty()) {
        channels.push_back(currentChannel);
    }

    for (size_t i = 0; i < channels.size(); ++i) {
        std::string& chan = channels[i];

        if (chan[0] != '#' && chan[0] != '&') {
            Utils::writeMessage(cli._cliFd, "Invalid channel name: " + chan + "\r\n");
            continue;
        }

        if (isChannelExist(chan)) {
            for (chanIt it = _channels.begin(); it != _channels.end(); ++it) {
                if (it->_name == chan) {
                    for (cliIt it2 = it->_channelClients.begin(); it2 != it->_channelClients.end(); ++it2) {
                        if (it2->_nick == cli._nick) {
                            Utils::writeMessage(cli._cliFd, RPL_PART(cli._nick, chan));
                            it->_channelClients.erase(it2);
                            if (it->_channelClients.size() > 0)
                                it->_opNick = it->_channelClients[0]._nick;
                            std::cout << RED << "Client " << cli._nick << " has left channel " << chan << RESET << std::endl;
                            break;
                        }
                    }
                    if (it->_channelClients.size() == 0) {
                        std::cout << RED << "Channel " << it->_name << " is deleted" << RESET << std::endl;
                        _channels.erase(it);
                    } else {
                        showRightGui(cli, *it);
                    }
                    break;
                }
            }
        } else {
            Utils::writeMessage(cli._cliFd, ERR_NOSUCHCHANNEL(chan, cli._nick));
        }
    }
}