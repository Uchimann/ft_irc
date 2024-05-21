#include "../includes/Server.hpp"
#include "../includes/Utils.hpp"

Server *Server::singleton = NULL;
Server::Server() : _botFd(0), _fdCount(0) {}

Server::~Server()
{
    if (singleton != NULL)
        delete singleton;
    singleton = NULL;
    close(_serverFd);
}

Server *Server::getInstance()
{
    try
    {
        if (singleton == NULL)
            singleton = new Server;
        return singleton;
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        exit(1);
    }
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

// socket fonksiyonu, yeni bir soket oluşturur.
// AF_INET parametresi, IPv4 adres ailesini,
// SOCK_STREAM ise TCP protokolünü belirtir.
// Eğer soket oluşturma işlemi başarısız olursa, -1 döner ve runtime_error ile bir hata fırlatılır.
// fcntl fonksiyonu, soketi blok olmayan hale getirir. Bu, soketin non-blocking olarak çalışmasını sağlar.
// Bu, aynı zamanda I/O işlemlerinin bloke olmadan devam etmesine olanak tanır.

// setsockopt fonksiyonu, soket seçeneklerini ayarlar. Bu durumda, SO_REUSEADDR seçeneği, aynı adresi ve portu birden fazla soketle paylaşabilme yeteneğini sağlar.
// Bu, sunucunun hızlı yeniden başlatılmasını veya bağlantı hatası durumunda aynı portu hızlı bir şekilde kullanabilmesini sağlar.
void Server::createSocket()
{
    if ((_serverFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) // (ipv4, tcp) protokolü ile yeni bir socket oluşturuyoruz
        throw std::runtime_error("Socket");
    fcntl(_serverFd, F_SETFL, O_NONBLOCK); // soketi nonblock hale getirir. F_SETFL, dosya durumunu ayarlamak için kullanılır.
    const int enable = 1;
    if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        throw std::runtime_error("Setsockopt");
}

void Server::bindSocket(size_t const &port)
{
    struct sockaddr_in server_addr; // IPv4 adresleri için bir adres yapısı tanımlanır.

    memset(&server_addr, 0, sizeof(server_addr)); // server addr icini 0 ile doldurur (sıfırlar null doldurur)
    server_addr.sin_addr.s_addr = INADDR_ANY;     // Sunucunun bağlanacağı IP adresini belirler. INADDR_ANY tüm mevcut ağ arabirimlerine bağlanmayı ifade eder.
    server_addr.sin_family = AF_INET;             // Adres ailesini belirtir, bu durumda AF_INET IPv4 adres ailesini belirtir.
    server_addr.sin_port = htons(port);           // Bağlanılacak port numarasını belirtir. htons fonksiyonu, host byte order'ını network byte order'a dönüştürür (endianness konusunda uyumsuzlukları düzeltir).
    // yani htons sayesinde gönderici, gödnerdiği bit sıralamasını alıcının sbit sıralama tipine göre hazırlayıp göndermeye yarıyor.
    if (bind(_serverFd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) // soketi belirtilen adres ve port ile ilişkilendirir.
        throw std::runtime_error("Bind");

    if (listen(_serverFd, SOMAXCONN) < 0) // soketi bir bağlantı noktasına dönüştürür ve gelen bağlantıları dinlemeye başlar. SOMAXCONN parametresi, aynı anda dinlenebilecek en fazla bağlantı sayısını belirtir.
        throw std::runtime_error("Listen");
}

void Server::acceptRequest()
{
    Client tmp;
    sockaddr_in cliAddr;                 // Bu yapı, istemci bağlantısının adres ve bağlantı noktası bilgilerini tutar.
    socklen_t cliSize = sizeof(cliAddr); // cliAddr yapısının boyutu bu değişkene atanır. Bu, sonradan accept fonksiyonuna geçilecek.

    // Yeni bir istemci bağlantısı kabul edilir. accept fonksiyonu, gelen bağlantıyı kabul eder ve istemcinin dosya tanımlayıcısını döndürür.
    // Bu dosya tanımlayıcısı, istemci ile iletişim kurmak için kullanılır.
    tmp._cliFd = accept(_serverFd, (sockaddr *)&cliAddr, &cliSize); // burda accept fonkyisonu isteği kabul eder ve istemcinin fd sini döndürür.

    fcntl(tmp._cliFd, F_SETFL, O_NONBLOCK); // Alınan istemci dosya tanımlayıcısını engelsiz hale getirir.
    if (tmp._cliFd <= 0)                    // bağlantı başarısız olursa kontrolu
        throw std::runtime_error("Accept failed");

    // İstemcinin bağlandığı bağlantı noktası bilgisi alınır ve geçici Client nesnesinin _port özelliğine atanır.
    tmp._port = ntohs(cliAddr.sin_port);

    // İstemcinin IP adresi alınır ve ASCII dize biçimine dönüştürülür. Buralara detaylı bakılacak - uchiman -emre
    inet_ntop(AF_INET, &(cliAddr.sin_addr), tmp._ipAddr, INET_ADDRSTRLEN);

    // Yeni istemcinin dosya tanımlayıcısı, sunucunun okuma dosya tanımlayıcı kümesine eklenir. Bu, gelecekteki okuma işlemlerini izlemek için gereklidir.
    FD_SET(tmp._cliFd, &_readFds);

    std::cout << GREEN << "New client connected!" << RESET << std::endl;
    _fdCount++;
    _clients.push_back(tmp); // Geçici Client nesnesi, sunucunun istemci listesine eklenir.
}

// Açıkama fonksiyon altında - yucOx
/*std::map<std::string, std::vector<std::string>> Server::getParams(std::string const &str)
{
    std::stringstream ss(str);
    std::string tmp;
    std::map<std::string, std::vector<std::string>> ret;
    std::vector<std::string> params;
    ss >> tmp;
    std::string cmd;
    while (1)
    {
        cmd = tmp;
        if (ret.find(cmd) != ret.end())
            return ret;
        params.clear();
        ss >> tmp;
        while (_commands.find(tmp) == _commands.end())
        {
            params.push_back(tmp);
            ss >> tmp;
            if (ss.eof())
            {
                ret[cmd] = params;
                return ret;
            }
        }
        if (ss.eof())
        {
            params.push_back("");
            ret[cmd] = params;
            return ret;
        }
        if (params.empty())
            params.push_back("");
        ret[cmd] = params;
    }
    return ret;
}*/


// Üstteki fonksiyonda değişken isimleri kötü isimlendirimiş ve gereksiz tekrarları var aşağıdaki hali
// ile güncellendi, problem halinde eskiye dönülebilir -yucOx

// Bu fonksiyon, gelen veriyi işler ve komutları ve parametreleri ayırır (detayları altta) -yucOx
std::map<std::string, std::vector<std::string>> Server::getParams(const std::string &input)
{
    std::stringstream inputStream(input);
    std::string currentWord;
    std::map<std::string, std::vector<std::string>> commandParamsMap;
    std::vector<std::string> paramsList;
    inputStream >> currentWord;
    std::string currentCommand;
    while (true)
    {
        currentCommand = currentWord;
        if (commandParamsMap.find(currentCommand) != commandParamsMap.end()) //
            return commandParamsMap;
        paramsList.clear();
        inputStream >> currentWord;
        
        while (_commands.find(currentWord) == _commands.end()) // komutlar mapinde olmayan bir kelimeye rastlanırsa 
        // ör: 1.parametre NICK 2.parametre boş ya da 1.girdi dolu 'anlamlı anlamsız fark etmez'  -yucOx 
        {
            paramsList.push_back(currentWord);
            inputStream >> currentWord;
            if (inputStream.eof())
            {
                commandParamsMap[currentCommand] = paramsList;
                return commandParamsMap;
            }
        }
        // Eğer 1.girdi varsa, anlamlıysa -> 'bot' ve 2.girdi boş ise buraya girer. -yucOx
        if (inputStream.eof())
        {
            paramsList.push_back("");
            commandParamsMap[currentCommand] = paramsList;
            return commandParamsMap;
        }
        // Eğer 1.girdi varsaş anlamlıysa -> 'bot' v.b ve 2.girdi de anlamlıysa 'bot' v.b buraya girer. -yucOx
        if (paramsList.empty()){
            paramsList.push_back("");
        }
        commandParamsMap[currentCommand] = paramsList;
    }
    return commandParamsMap;
} // !!!alternatif olarak üstteki fonksiyon yerine alttaki fonksiyon kullanılabilir size bırakıyorum -yucOx
/*
std::map<std::string, std::vector<std::string> > Server::getParams(const std::string &input)
{
    std::stringstream inputStream(input);
    std::string currentWord;
    std::map<std::string, std::vector<std::string> > commandParamsMap;
    std::vector<std::string> paramsList;
    std::string currentCommand;

    // İlk komutu oku
    if (!(inputStream >> currentWord)) {
        return commandParamsMap; // Eğer giriş boşsa, boş bir harita döner.
    }

    while (true)
    {
        currentCommand = currentWord;
        paramsList.clear();

        // Parametreleri topla
        while (inputStream >> currentWord && _commands.find(currentWord) == _commands.end())
        {
            paramsList.push_back(currentWord);
        }

        // Eğer giriş akışının sonuna geldiysek veya yeni bir komut bulduysak, mevcut komutu haritaya ekle
        commandParamsMap[currentCommand] = paramsList;

        // Giriş akışının sonuna gelindiyse, döngüyü sonlandır
        if (inputStream.eof()) {
            break;
        }
        
        // Yeni bir komut bulundu, döngü devam eder
    }

    return commandParamsMap;
}
*/



void Server::commandHandler(std::string &str, Client &cli)
{
    std::map<std::string, std::vector<std::string>> params = getParams(str); // command - parametreler şeklinde gelen stringi işler ve komutu ve parametreleri ayırır -yucOx
    for (std::map<std::string, std::vector<std::string>>::iterator it = params.begin(); it != params.end(); ++it)
    {
        if (_commands.find(it->first) == _commands.end())
        {
            Utils::writeMessage(cli._cliFd, "421 : " + it->first + " :Unknown command!\r\n");
            std::cout << RED << it->first << " command not found!" << RESET << std::endl;
        }
        else
            (this->*_commands[it->first])(it->second, cli);
    }
}

// Sunucunun bir veya daha fazla istemciden gelen verileri okumasını sağlar.
void Server::readEvent(int *state)
{
    for (cliIt it = _clients.begin(); it != _clients.end(); ++it)
    {                                           // _clients vektöründe yer alan her istemci için bir döngü başlatır.
        if (FD_ISSET(it->_cliFd, &_readFdsSup)) // İlgili istemcinin dosya tanımlayıcısı, okuma dosya tanımlayıcı kümesinde mi kontrol edilir.
        {                                       // Eğer istemcinin dosya tanımlayıcısı okuma işlemi için hazırsa, bu koşul sağlanır.
            *state = 0;
            int readed = read(it->_cliFd, _buffer, 1024); // read fonksiyonu, belirtilen dosya tanımlayıcısından veri okur ve _buffer dizisine yerleştirir.
            if (readed <= 0)
            {                                         // Eğer okunan veri yoksa veya bir hata oluşmuşsa
                std::vector<std::string> tmp;         //
                tmp.push_back("");                    // Geçici bir boş dize vektörü oluşturulur.
                (this->*_commands["QUIT"])(tmp, *it); // "QUIT" komutu işlenir. Bu, ilgili istemcinin sunucudan çıkmasını sağlar.
            }
            else // Eğer veri okunduysa, bu blok çalışır.
            {
                _buffer[readed] = 0;       // okunan verinin sonuna null eklenir. böylece bu veri bir C-string oluşturulur.
                std::string tmp = _buffer; // buffer dizisi stringe dönüştürülür.
                if (tmp == "\n")
                { // ğer okunan veri sadece yeni satır karakteri ise, bu durum işlenir ve state sıfırlanır, böylece döngü sona erer.
                    std::cout << "readevent icinde yeni satır karakteri geldi\n" << std::endl; // -uchiman burasini kontrol edeceğim
                    *state = 0;
                    break;
                }

                // Eğer okunan verinin sonunda yeni satır karakteri yoksa, bu veri bir sonraki okuma için biriktirilir ve döngü sona erer.
                /*if (tmp[tmp.length() - 1] != '\n')
                {
                    it->_buffer += tmp;
                    *state = 0;
                    break;
                }*/ //!!!Böyle bir ihtimal yok. -yucOx 

                // eğer okunan verinin sonunda yeni satır karakteri varsa, bu veri işlenir.
                else
                {
                    it->_buffer = it->_buffer + tmp;
                }
                std::cout << YELLOW << it->_buffer << RESET; // Konsola, alınan veri yazdırılır.
                commandHandler(it->_buffer, *it);            // Alınan komut işlenir ve ilgili işlev çağrılır.
                it->_buffer.clear();                         // İstemcinin veri biriktirme tamponu temizlenir.
            }
            break;
        }
        // kendime not: emre ile beraber buralarda bir yerlerde read edilen kısımları kontrol et mesaj tam alınmış mı diye (bosluklarla beraber) -uchiman
    }
}

// FD_ZERO makrosu, verilen dosya tanımlayıcı kümesini boşaltır,

// FD_SET sunucu soketinin dosya tanımlayıcı kümesine eklenmesini sağlar. _serverFd, sunucunun ana soketinin dosya tanımlayıcısını temsil eder.
// Bu, sunucunun gelen bağlantıları kabul edeceği ve dinleyeceği soketi belirtir.
void Server::initFds()
{
    FD_ZERO(&_readFds);
    FD_ZERO(&_writeFds);
    FD_ZERO(&_readFdsSup);
    FD_ZERO(&_writeFdsSup);
    FD_SET(_serverFd, &_readFds); // önce kümelerin içini boşalttık sonra da serverfd fd degerini readfds kümesine ekledik. Şuan readfds kümesinde bir tane filedescp. bulunuyor
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

/*
 select fonlsiyonu:
 int nfds : fd_set türü ile oluşturduğumuz soket kümesi içinde bulunan soketlerin en büyük fd değerine sahip soketin fd numarasının bir fazlasını arguman olarak alır.
 bizim burda fdcount +4 dememizin sebebi şudur. mininmum fd sayımız 3 tür ve 1 fazlsı 4 olur. başlangıçta kümenin içinin boş olduğunu varsayarsak, +1 dediğimizde sıfırın 1
 üstü olan 1 sayısını ele alacaktı. Bu durumda 1 (yani özel olan fd sayısını ele alıcaktık). Bu durumun önüne geçmek için + 4 yaptık ki. 0+4 den ilk fd değeri 4 ıolsun
 yukarısı benim tahminimdir kesinliğine bakacağım - Uchiman
*/
void Server::run()
{
    int state = 0;

    initFds(); // dosya tanımlayıcılarını (file descriptors) başlatır. Sunucunun giriş ve çıkış işlemleri için gereken kaynakları ayarlar.
    while (1)
    {
        _readFdsSup = _readFds;
        _writeFdsSup = _writeFds;
        // yukarıdaki işlem okuma ve yazma dosya tanımlayıcılarını yedekler. Bu, orijinal dosya tanımlayıcılarının durumunu korur.
        // yani selecte kopyayı göndercez orjinali bozmasın diye
        state = select(_fdCount + 4, &_readFdsSup, &_writeFdsSup, NULL, 0);
        // Select çağrısı ile gelen bağlantıları ve yazma işlemlerini kontrol eder. select, bekleyen bir giriş, çıkış veya hata durumu olup olmadığını kontrol eder.

        // select'e ek not: son parametreyi 0 yaptığımız için select nonblock çalışıcak. NULL yapsaydık select fonksiyonu olay gerçekleşene kadar bekleyecekti.

        /*ÖZETLE: Bu parametre, select fonksiyonunun en fazla ne kadar süre boyunca bekleyeceğini belirler.
         timeout argümanı, struct timeval türünde bir yapıdır ve iki alan içerir: tv_sec (saniye) ve tv_usec (mikrosaniye).
         Eğer bu argüman NULL olarak atanırsa, select fonksiyonu olay meydana gelene kadar bekler.
         Eğer 0 olarak atanırsa, fonksiyon anında kontrol sonuçlarını alır (bloklanmaz).
         Belirli bir zaman aralığı belirlenmişse, fonksiyon olay meydana gelene kadar veya belirtilen zaman aralığı sona erene kadar bekler
        Sunucu soketi üzerinde yeni bir bağlantı varsa, acceptRequest fonksiyonunu çağırır. Ardından, state'i 0'a ayarlar ve döngünün başına döner.*/

        if (FD_ISSET(_serverFd, &_readFdsSup))
        {                    // FD_ISSET makrosu, belirtilen fd'nin set kümesinde bulunup bulunmadığını kontrol eder.
            acceptRequest(); // Eğer fd kümede bulunuyorsa ve belirli bir olaya hazır durumdaysa, FD_ISSET true değerini döndürür;
            state = 0;
            continue; // acceptRequest fonksiyonunu çağırır ve döngünün başına döner.
        }
        if (state)
        {
            readEvent(&state);
            if (state == 0)
                continue;
        }
        // ****ALTTAKİ STATE'E HİÇBİR ŞEKİLDE GİRMİYOR, GERİ KALAN KISIMLAR TAMAM. BU KISIM ELLERİNİZDEN ÖPER o7****
        //Geri kalan kısımların çoğuna -yucOx etiketiyle not düştüm bakarsınız. şimdiden kolay gelsin -yucOx
        // soketler arası iletisimde alttaki bloğa giriyor. örneğin kullanıcılar arası mesajlarda veya kanallara yazılacak olan editlerde vs giriyor
        // fakat yazma veya okuma kısımlarında birkaç eksik var : işareti olma ve olmama durumlarına göre eksik yazıyor.
        if (state)
        {
            std::cout << "state: write kisminda " << std::endl;
            writeEvent();
            state = 0;
            continue;
        }
    }
}

void Server::manageServer(size_t const &port, std::string const &password)
{
    setPort(port);         // port ayarladık
    setPassword(password); // password ayarlandı
    initCommands();        // commands mapinin içine komutlara karşılık gelen fonksiyonların adresi verildi
    createSocket();        // soket oluşturuldu
    bindSocket(port);      // oluşturulan soketi belirtilen bir port numarasına bağlar ve bağlantıları dinlemeye başlar
    printStatus();         // ekrana server durumunu yazdırıyor
    run();                 //
}
