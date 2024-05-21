#include "../includes/Server.hpp"

int main(int ac, char **av)
{
    try {
        if (ac != 3)
            throw std::runtime_error("./ircserv <port> <password>");
        if (!Utils::portIsValid(av[1]))
            throw std::runtime_error("invalid port");
        Server::getInstance()->manageServer(atoi(av[1]), av[2]);// server classını gösteren pointer'ın manage server methodunu calıstırıyor
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
    // prıvmsg ve notıce konutlarında okuma veya pars kısımlarındabirkaç yerde değişiklik yapmamız lazım. 
    // ":" işareti olmadan gönderilen mesajların boşluklar ve sonraları gitmiyor. 
    // ":" işareti ile gönderilen mesaj ve noticelerde bosluklar ile beraber tüm mesajlar gidiyor.
    //getafter colon fonksiyonlarından mnce ve sonraki yerlerde bir kontrol hatasi olmuş olabilir. 
}
