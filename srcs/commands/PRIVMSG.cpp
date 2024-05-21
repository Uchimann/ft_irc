#include "../../includes/Server.hpp"

void Server::toChannel(std::vector<std::string>& params, Client& cli)
{
    if (isChannelExist(params[0]) == 0) {
        Utils::writeMessage(cli._cliFd, ERR_NOSUCHCHANNEL(params[0], params[1]));
        return ;
    }
    if (clientIsInThere(cli, params[0]) == 1)
    {
        std::cout << "getAfterColon kismina girmeden önceki if bloğundan önceki yer prıvmsg" << std::endl;
        if (params[1][0] == ':') {
            std::cout << "getAfterColon kismina girmeden önce prıvmsg" << std::endl;
            getAfterColon(params);
        }
        for (cliIt it = _clients.begin(); it != _clients.end(); ++it) {
            if (it->_nick != cli._nick && clientIsInThere(*it, params[0]) == 1) {
                it->_messageBox.push_back(RPL_PRIVMSG(cli._nick, params[0], params[1]));
                FD_SET(it->_cliFd, &_writeFds);
            }
        }
    }
}

void Server::toClient(std::vector<std::string>& params, Client& cli)
{
    int flag = 0;
    
    /*std::cout << "params 0 " << params[0] << std::endl;
        std::cout << "params 1 " << params[1] << std::endl;
            std::cout << "params 2 " << params[2] << std::endl;
                std::cout << "params 3 " << params[3] << std::endl;*/

    std::cout << "toClient kisminda 1.yer" << std::endl;
    for (cliIt it = _clients.begin(); it != _clients.end(); ++it) {            // burada mesajin ilk parametresinden sonraki parametrelerin birlestirilmesi gerekiyor
            std::cout << "toClient kisminda 2.yer" << std::endl;               // burakaya bakacağım -uchiman
        if (params[0] == it->_nick) {                                          // tamamdır galiba anladim. privmsg atarken : olmadan attığımızda ilk kelimeden sonrasi gitmiyor
                std::cout << "toClient kisminda 3.yer" << std::endl;           // : attığında herşey düzgün gidiyor get colona gidiyor önce.. Demek ki get after colonda doğru bişey var 
            if (params[1][0] == ':') {                                         // o doğru şeyi buradaki forun son kısımlarında bir yere koymamız gerekiyor
                    std::cout << "toClient kisminda 4.yer getaftercolon kismi" << std::endl;
               getAfterColon(params);
            } // burda sunu deneyeceğim. : ile mesaj atacağım ve bu if bloğunun dışına çıkıyor mu vs kontrol edeceğim - uchiman veya yukarıdaki param yazdırma işlemlerini bu ifin altındaki yerlerde deneyeceğim. - uchiman
            it->_messageBox.push_back(RPL_PRIVMSG(cli._nick, params[0], params[1]));
            FD_SET(it->_cliFd, &_writeFds);
            flag = 1;
            return ;
        }
    }
    if (flag == 0)
        Utils::writeMessage(cli._cliFd, ERR_NOSUCHNICK);
}

void Server::Privmsg(std::vector<std::string>& params, Client& cli)
{
    passChecker(cli);
    if (params.size() < 2)
    {
        Utils::writeMessage(cli._cliFd, ERR_NEEDMOREPARAMS(cli._nick, params[0]));
        return ;
    }
    else if (params[0][0] == '#')
        toChannel(params, cli);
    else
        toClient(params, cli);
}
