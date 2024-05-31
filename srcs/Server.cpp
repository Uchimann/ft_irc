#include "../includes/Server.hpp"
#include "../includes/Utils.hpp"

Server *Server::singleton = NULL;
bool Server::signal = false;

Server::Server() : _botFd(0), _fdCount(0) {}

void Server::clear_server()
{
    if (singleton != NULL)
    {
        delete singleton;
        singleton = NULL;
    }
    std::cout << RED << "Server is closed!" << RESET << std::endl;
}

Server *Server::getInstance()
{
    if (singleton == NULL)
    {
        try
        {
            singleton = new Server;
        }
        catch (const std::bad_alloc &ex)
        {
            std::cerr << "Error: Failed to allocate memory for Server instance: " << ex.what() << std::endl;
            singleton = NULL;
        }
    }
    return singleton;
}

void Server::signalHandler(int signum)
{
    (void)signum;
    Server::signal = true;
}

void Server::initCommands()
{
    _commands["PASS"] = &Server::Pass;
    _commands["NICK"] = &Server::Nick;
    _commands["JOIN"] = &Server::Join;
    _commands["CAP"] = &Server::Cap;
    _commands["USER"] = &Server::User;
    _commands["WHO"] = &Server::Who;
    _commands["QUIT"] = &Server::Quit;
    _commands["PART"] = &Server::Part;
    _commands["INFO"] = &Server::Info;
    _commands["PRIVMSG"] = &Server::Privmsg;
    _commands["WHOIS"] = &Server::Whois;
    _commands["whois"] = &Server::Whois;
    _commands["NOTICE"] = &Server::Notice;
    _commands["KICK"] = &Server::Kick;
    _commands["MODE"] = &Server::Mode;
    _commands["mode"] = &Server::Mode;
    _commands["TOPIC"] = &Server::Topic;
    _commands["topic"] = &Server::Topic;
    _commands["LIST"] = &Server::List;
    _commands["INVITE"] = &Server::Invite;
    _commands["OPER"] = &Server::Oper;
    _commands["bot"] = &Server::Bot;
    _commands["HELPME"] = &Server::Help;
}

void Server::createSocket()
{
    if ((_serverFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        throw std::runtime_error("Socket error!");
    fcntl(_serverFd, F_SETFL, O_NONBLOCK);
    const int enable = 1;
    if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        throw std::runtime_error("Setsockopt error!");
}

void Server::bindSocket(size_t const &port)
{
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    //htons -> host to network short
    if (bind(_serverFd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        throw std::runtime_error("Bind error! Maybe port is already in use.");

    if (listen(_serverFd, SOMAXCONN) < 0)
        throw std::runtime_error("Listen");
}

void Server::acceptRequest()
{
    Client tmp;
    sockaddr_in cliAddr;
    socklen_t cliSize = sizeof(cliAddr);


    tmp._cliFd = accept(_serverFd, (sockaddr *)&cliAddr, &cliSize);

    fcntl(tmp._cliFd, F_SETFL, O_NONBLOCK);
    if (tmp._cliFd <= 0)                    
        throw std::runtime_error("Accept failed");

    tmp._port = ntohs(cliAddr.sin_port);
    inet_ntop(AF_INET, &(cliAddr.sin_addr), tmp._ipAddr, INET_ADDRSTRLEN);
    FD_SET(tmp._cliFd, &_readFds);

    std::cout << GREEN << "New client connected!" << RESET << std::endl;
    _fdCount++;
    _clients.push_back(tmp);
}

std::map<std::string, std::vector<std::string> > Server::getParams(const std::string &input)
{
    std::stringstream inputStream(input);
    std::string currentWord;
    std::map<std::string, std::vector<std::string> > commandParamsMap;
    std::vector<std::string> paramsList;
    inputStream >> currentWord;
    std::string currentCommand;
    while (true)
    {
        currentCommand = currentWord;
        if (commandParamsMap.find(currentCommand) != commandParamsMap.end())
            return commandParamsMap;
        paramsList.clear();
        inputStream >> currentWord;
        
        while (_commands.find(currentWord) == _commands.end())
        {
            paramsList.push_back(currentWord);
            inputStream >> currentWord;
            if (inputStream.eof())
            {
                commandParamsMap[currentCommand] = paramsList;
                return commandParamsMap;
            }
        }
        if (inputStream.eof())
        {
            paramsList.push_back("");
            commandParamsMap[currentCommand] = paramsList;
            return commandParamsMap;
        }
        if (paramsList.empty()){
            paramsList.push_back("");
        }
        commandParamsMap[currentCommand] = paramsList;
    }
    return commandParamsMap;
}

void Server::commandHandler(std::string &str, Client &cli)
{
    std::map<std::string, std::vector<std::string> > params = getParams(str);
    for (std::map<std::string, std::vector<std::string> >::iterator it = params.begin(); it != params.end(); ++it)
    {
        if (_commands.find(it->first) == _commands.end())
        {
            Utils::writeMessage(cli._cliFd, "421 : " + it->first + " :Unknown command!\r\n");
            std::cout << RED << it->first << " command not found!" << RESET << std::endl;
        }
        else
        {
            (this->*_commands[it->first])(it->second, cli);
        }
    }
}


void Server::readEvent(int *state)
{
    for (cliIt it = _clients.begin(); it != _clients.end(); ++it)
    {                                          
        if (FD_ISSET(it->_cliFd, &_readFdsSup))
        {                                       
            *state = 0;
            int readed = read(it->_cliFd, _buffer, 1024);
            if (readed <= 0)
            {                                       
                std::vector<std::string> tmp;         
                tmp.push_back("");                    
                (this->*_commands["QUIT"])(tmp, *it); 
            }
            else 
            {
                _buffer[readed] = 0;
                std::string tmp = _buffer; 
                if (tmp == "\n")
                { 
                    *state = 0;
                    break;
                }
                if (tmp[tmp.length() - 1] != '\n')
                {
                    it->_buffer += tmp;
                    *state = 0;
                    break;
                }
                else 
                {
                    it->_buffer = it->_buffer + tmp;
                }
                std::cout << YELLOW << it->_buffer << RESET;
                commandHandler(it->_buffer, *it);   
                it->_buffer.clear();                         
            }
            break;
        }
    }
}

void Server::initFds()
{
    FD_ZERO(&_readFds);
    FD_ZERO(&_writeFds);
    FD_ZERO(&_readFdsSup);
    FD_ZERO(&_writeFdsSup);
    FD_SET(_serverFd, &_readFds);
}

void Server::writeEvent()
{
    for (cliIt it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (FD_ISSET(it->_cliFd, &_writeFdsSup))
        {
            int writed = write(it->_cliFd, it->_messageBox[0].c_str(), it->_messageBox[0].size());
            it->_messageBox.erase(it->_messageBox.begin());
            if (it->_messageBox.empty())
                FD_CLR(it->_cliFd, &_writeFds);
            if (writed <= 0)
            {
                std::vector<std::string> tmp;
                tmp.push_back("");
                (this->*_commands["QUIT"])(tmp, *it);
            }
            break;
        }
    }
}

void Server::run()
{
    int state = 0;

    initFds();
    while (!Server::signal)
    {
        _readFdsSup = _readFds; 
        _writeFdsSup = _writeFds;
        state = select(_fdCount + 4, &_readFdsSup, &_writeFdsSup, NULL, 0);

        if (FD_ISSET(_serverFd, &_readFdsSup))
        {                    
            acceptRequest();
            state = 0;
            continue; 
        }
        if (state)
        {
            readEvent(&state);
            if (state == 0)
                continue;
        }
        if (state)
        {
            writeEvent();
            state = 0;
            continue;
        }
    }
}

void Server::manageServer(size_t const &port, std::string const &password)
{
    setPort(port);  
    setPassword(password);
    initCommands();
    createSocket();
    bindSocket(port);
    printStatus();
    run();
}
