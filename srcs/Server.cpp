#include "../includes/Server.hpp"

Server::Server(int port, std::string pwd)
{
    _pwd = pwd;
    _serv_socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (_serv_socket_fd < 0)
        throw std::runtime_error("Error : socket failed");
    push_back_fd(_serv_socket_fd, POLLIN);
    
    memset(&_serv_addr, 0, sizeof(_serv_addr));
    _serv_addr.sin_family = AF_INET;
    _serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    _serv_addr.sin_port = htons(port);

    if (bind(_serv_socket_fd, (struct sockaddr *)&_serv_addr, sizeof(_serv_addr)) == -1)
        throw std::runtime_error("Error : bind failed");

    if (listen(_serv_socket_fd, 42) == -1)
        throw std::runtime_error("Error : listen failed");
}

Server::~Server()
{
    close(_serv_socket_fd);
    exit(0);
}

void Server::push_back_fd(int fd, short event)
{
    struct pollfd   s_fd;

    s_fd.fd = fd;
    s_fd.events = event;
    _pollfd_vec.push_back(s_fd);
}

void Server::close_fd(int fd)
{
    pollfd_vec::iterator it;

    for (it = _pollfd_vec.begin(); it != _pollfd_vec.end(); ++it)
    {
        if ((*it).fd == fd)
        {
            _pollfd_vec.erase(it);
            break;
        }
    }
    close(fd);
}

void Server::server_run()
{
    int prepared_fd;

    while (1)
    {
        prepared_fd = poll(_pollfd_vec.data(), _pollfd_vec.size(), 1000);
        std::cout << "prepared_fd : " << prepared_fd << std::endl;
        while (prepared_fd == 0) {
            prepared_fd = poll(_pollfd_vec.data(), _pollfd_vec.size(), 1000);
        }
        if (prepared_fd < 0)
            throw std::runtime_error("Error : poll failed");
        // server fd
        if (_pollfd_vec[0].revents & POLLIN)
            accept_client();
        // client fd
        for (pollfd_vec::iterator iter = _pollfd_vec.begin() + 1; iter < _pollfd_vec.end(); ++iter)
        {
            std::cout << " : " << (*iter).revents << std::endl;
            // client disconnect
            if (iter->revents & POLLHUP)
            {
                Command command("QUIT :leaving");
                command.cmd_quit(*this, (*iter).fd);
                --iter;
                continue;
                /* 구현 해야함 */
                // QUIT command 를 사용하면 어떨까
            }
            if (iter->revents & POLLNVAL)
            {
                delClient((*iter).fd);
                --iter;
                continue;
            }
            // client receive
            if (iter->revents & POLLIN)
            {
                if (!recv_message((*iter).fd))
                    --iter;
            }
        }
        std::cout << "여기니 3" << std::endl;
    }
}

void Server::accept_client()
{
    int client_fd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    try {
        // accept 연결요청의 수락을 위한 함수 (연결요청이 있을 때까지 함수는 반환하지 않음)
        client_fd = accept(_serv_socket_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0)
            throw std::runtime_error("Error : accept failed");
        fcntl(client_fd, F_SETFL, O_NONBLOCK);
        push_back_fd(client_fd, POLLIN);
    } catch (std::exception& e) {
		std::cerr << e.what() << "\n";
		close(client_fd);
    }
}

int Server::recv_message(int fd)
{
    try {
        std::string msg = get_message(fd);
        if (!execute_command(msg, fd))
            return (0);
    } catch (std::exception& e) {
		std::cerr << e.what() << "\n";
        Command command("QUIT");
        command.cmd_quit(*this, fd);
        return (0);
    }
    return (1);
}

void Server::send_message(std::string const& msg, int c_fd)
{
    int len;

    try {
        std::cout << msg << std::endl;
        len = send(c_fd, msg.c_str(), msg.length(), 0);
        if (len < 0)
            throw std::runtime_error("Error : send failed");
        if (len == 0)
            throw std::runtime_error("Closed socket");
        // len 만큼 다 보내졌는지 확인 해야한다고 함
        // if (len != msg.length()) 
        //     len = send()
    } catch (std::exception& e) {
        std::cerr << e.what() << "\n";
        Command command("QUIT");
        command.cmd_quit(*this, c_fd);
    }
}

// :test!ubuntu@127.0.0.1 MODE #hi :-it
void Server::send_message(std::string const& cmd, std::string const& msg, Channel &channel, int c_fd)
{
    int len;
    Client client = getClient(c_fd);
    users_vec users = channel.get_users();
    users_vec opers = channel.get_opers();
    users_vec::iterator it = opers.end();
    std::string message;

    try
    {
        for (it = opers.begin(); it < opers.end(); ++it)
        {
            if ((*it)->get_fd() == c_fd) // 여기서 터짐
                continue;
            message = ":" + client.get_nick() + "!" + client.get_user() + "@" + client.get_host() + " "
                        + cmd + " " + channel.get_name() + " " + msg + "\r\n";
            len = send((*it)->get_fd(), message.c_str(), message.length(), 0);
            if (len < 0)
                throw std::runtime_error("Error : send failed");
            if (len == 0)
                throw std::runtime_error("Closed socket");
            // if (len < 0)
            //     throw std::runtime_error("Error : send failed");
            // len 만큼 다 보내졌는지 확인 해야한다고 함
            // if (len != msg.length()) 
            //     len = send()
            message = "";
        }
        for (it = users.begin(); it != users.end(); ++it)
        {
            if ((*it)->get_fd() == c_fd)
                continue;
            message = ":" + client.get_nick() + "!" + client.get_user() + "@" + client.get_host() + " "
                        + cmd + " " + channel.get_name() + " " + msg + "\r\n";
            len = send((*it)->get_fd(), message.c_str(), message.length(), 0);
            if (len < 0)
                throw std::runtime_error("Error : send failed");
            if (len == 0)
                throw std::runtime_error("Closed socket");
            // if (len < 0)
            //     throw std::runtime_error("Error : send failed");
            // len 만큼 다 보내졌는지 확인 해야한다고 함
            // if (len != msg.length()) 
            //     len = send()
            message = "";
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << "\n";
		// close((*it)->get_fd);
    }
    
}

std::string Server::get_message(int fd)
{
    int len;
    std::string msg = "";
    // char _buf[BUF_SIZE]; \r\n 뒤에 남아있다면?? 우선 클래스 안으로 이동

    while ((len = recv(fd, _buf, BUF_SIZE, 0)) != 0) // 0 : EOF
    {
        if (len < 0)
            throw std::runtime_error("Error : recv failed");
        if (len == 0)
            throw std::runtime_error("Closed socket");
        _buf[len] = 0;
        msg += _buf;
        if (msg.length() >= 2 && !msg.substr(msg.length() - 2).compare("\r\n")) // \r\n
            break;
    }
    return (msg);
}

void Server::handle_err(int err_num, std::string err_msg, int c_fd)
{
    std::cerr << err_num << " " << err_msg << std::endl;
    send_message(err_msg, c_fd);
}

typedef std::vector<std::string>	str_vec;
str_vec Server::split_commands(std::string msg)
{
    str_vec commands;
    std::string message = msg;
    
    for (size_t pos = 0; pos != std::string::npos; )
    {
        pos = 0;
        pos = message.find("\r\n");
        commands.push_back(message.substr(0, pos));
        if (message.length() <= 2)
            message = "";
        else
            message = message.substr(pos + 2, message.length());
    }
    return (commands);
}


/*  구현중  */
int Server::execute_command(std::string msg, int c_fd)
{
    str_vec commands = split_commands(msg);
    str_vec::iterator it;

    for (it = commands.begin(); it < commands.end(); ++it)
    {
        Command cmd(*it);
        if (!cmd.classify_command(*this, c_fd))
            return (0);
    }
    return (1);
}

// 처음 들어왔을 때
    // client vec
// 계속 통신 하고 있는 앤지 

int Server::compare_pwd(std::string const& cpwd)
{
    return(cpwd.compare(_pwd));
}

void Server::save_client(Client& client)
{
    _client.push_back(client);
}

void Server::save_channel(Channel& channel)
{
    _channel.push_back(channel);
}

Client& Server::getClient(int c_fd)
{
    client_vec::iterator it;

    for (it = _client.begin(); it != _client.end(); ++it)
    {
        if (c_fd != it->get_fd())
            continue;
        else
            break;
    }
    return (*it);
}

Client& Server::getClient(std::string name)
{
    client_vec::iterator it;

    for (it = _client.begin(); it != _client.end(); ++it)
    {
        if (name != it->get_nick())
            continue;
        else
            break;
    }
    return (*it);
}

Channel& Server::getChannel(std::string name)
{
    channel_vec::iterator it;

    for (it = _channel.begin(); it != _channel.end(); ++it)
    {
        if (name != it->get_name())
            continue;
        else
            break;
    }
    return (*it);
}

int Server::is_client(int c_fd)
{
    client_vec::iterator it;

    for (it = _client.begin(); it < _client.end(); ++it)
    {
        if ((*it).get_fd() == c_fd)
            return 1;
    }
    return 0;
}

void Server::delClient(int c_fd)
{
    client_vec::iterator it;

    for (it = _client.begin(); it < _client.end(); ++it)
    {
        if((*it).get_fd() == c_fd)
        {
            _client.erase(it);
            return ;
        }
    }
    close_fd(c_fd);
}


void Server::delChannel(Channel& channel)
{
    channel_vec::iterator it;

    for (it = _channel.begin(); it < _channel.end(); ++it)
    {
        if((*it).get_name() == channel.get_name())
        {
            _channel.erase(it);
            return ;
        }
    }
}


int Server::is_valid_nick(int c_fd, std::string nick)
{
    client_vec::iterator it;

    (void)c_fd;
    for (it = _client.begin(); it < _client.end(); ++it)
    {
        if (it->get_nick() == nick) 
            return (0);
    }
    return (1);
}

int Server::is_valid_channel(std::string name)
{
    channel_vec::iterator it;

    for (it = _channel.begin(); it < _channel.end(); ++it)
    {
        if (it->get_name() == name) 
            return (0);
    }
    return (1);
}
