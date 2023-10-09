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

    _msg.resize(100, std::string(""));
    std::cout << "==========================================================\n";
    std::cout << "                 my little little ircserv\n";
    std::cout << "==========================================================\n";
}

Server::~Server()
{
    for (client_vec::iterator it = _client.begin(); it != _client.end(); ++it)
    {
        delete *it;
    }
    _client.clear();
    for (channel_vec::iterator it = _channel.begin(); it != _channel.end(); ++it)
    {
        delete *it;
    }
    _channel.clear();
    for (pollfd_vec::iterator it = _pollfd_vec.begin(); it != _pollfd_vec.end(); ++it)
    {
        close((*it).fd);
    }
    exit(0);
}

void Server::push_back_fd(int fd, short event)
{
    struct pollfd   s_fd;

    s_fd.fd = fd;
    s_fd.events = event;
    _pollfd_vec.push_back(s_fd);
}

void Server::server_run()
{
    int ret = 0;

    while (1)
    {
		ret = poll(_pollfd_vec.data(), _pollfd_vec.size(), 1000);
        if (ret < 0)
            throw std::runtime_error("Error : poll failed");
        // server fd
        if (_pollfd_vec[0].revents & POLLIN)
            accept_client();
        // client fd
        for (pollfd_vec::iterator iter = _pollfd_vec.begin() + 1; iter < _pollfd_vec.end(); ++iter)
        {
            // client receive
            if (iter->revents & POLLIN)
            {
                if (!recv_message((*iter).fd))
                {
                    iter = _pollfd_vec.erase(iter);
                    continue;
                }
            }
            // client disconnect
            if (iter->revents & POLLHUP || iter->revents & POLLNVAL)
            {
                Command command("QUIT :leaving");
                command.cmd_quit(*this, (*iter).fd);
                iter = _pollfd_vec.erase(iter);
                continue;
            }
        }
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
        if (msg == "")
            return (1);
        if (!execute_command(msg, fd))
            return (0);
    } catch (std::exception& e) {
		std::cerr << e.what() << "\n";
        Command command("QUIT");
        command.cmd_quit_closed(*this, fd);
        return (0);
    }
    return (1);
}

void Server::send_message(std::string const& msg, int c_fd)
{
    const char* message = msg.c_str();
    int len = msg.length();
    int total_len = 0;

    try {
        while (total_len < len) {
            int sent = send(c_fd, message + total_len, len - total_len, 0);
            if (sent == -1) {
                throw std::runtime_error("Error : send failed");
            }
            total_len += sent;
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << "\n";
        Command command("QUIT");
        command.cmd_quit(*this, c_fd);
    }
}

// :test!ubuntu@127.0.0.1 MODE #hi :-it
void Server::send_message(std::string const& cmd, std::string const& msg, Channel* channel, int c_fd)
{
    Client* client = getClient(c_fd);
    client_vec users = channel->get_users();
    client_vec opers = channel->get_opers();
    client_vec::iterator it = opers.end();
    client_vec::iterator u_it = users.end();
    std::string message;
    const char* c_message;
    int len;
    int total_len;

    try
    {
        for (it = opers.begin(); it < opers.end(); ++it)
        {
            len = 0;
            total_len = 0;
            if ((*it)->get_fd() == c_fd)
                continue;
            message = ":" + client->get_nick() + "!" + client->get_user() + "@" + client->get_host() + " "
                        + cmd + " " + channel->get_name() + " " + msg + "\r\n";
            c_message = message.c_str();
            len = message.length();
            while (total_len < len) {
                int sent = send((*it)->get_fd(), c_message + total_len, len - total_len, 0);
                if (sent == -1) {
                    throw std::runtime_error("Error : send failed");
                }
                total_len += sent;
            }
            message = "";
        }
        for (u_it = users.begin(); u_it < users.end(); ++u_it)
        {
            len = 0;
            total_len = 0;
            if ((*u_it)->get_fd() == c_fd)
                continue;
            message = ":" + client->get_nick() + "!" + client->get_user() + "@" + client->get_host() + " "
                        + cmd + " " + channel->get_name() + " " + msg + "\r\n";
            c_message = message.c_str();
            len = message.length();
            while (total_len < len) {
                int sent = send((*u_it)->get_fd(), c_message + total_len, len - total_len, 0);
                if (sent == -1) {
                    throw std::runtime_error("Error : send failed");
                }
                total_len += sent;
            }
            message = "";
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << "\n";
        Command command("QUIT");
        command.cmd_quit(*this, c_fd);
    }
    
}

std::string Server::get_message(int fd)
{
    int len = 0;
    std::string msg = _msg[fd];

    while ((len = recv(fd, _buf, BUF_SIZE, 0)) > 0) // 0 : EOF
    {
        _buf[len] = 0;
        msg += _buf;
        _msg[fd] += _buf;
        if (msg.length() >= 2 && !msg.substr(msg.length() - 2).compare("\r\n"))
        {
            std::cout << "\n[ Client " << fd << " ]\n";
            std::cout << msg << "\n";
            _msg[fd] = "";
            return (msg);
        }
    }
    if (len == 0)
        throw std::runtime_error("Closed socket");
    return ("");
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

int Server::compare_pwd(std::string const& cpwd)
{
    std::string pwd = cpwd;
    if (pwd.find("\n") != std::string::npos)
        pwd.erase(pwd.length() - 1);
    return(_pwd.compare(pwd));
}

void Server::save_client(Client& client)
{
    _client.push_back(&client);
}

void Server::save_channel(Channel& channel)
{
    _channel.push_back(&channel);
}

Client* Server::getClient(int c_fd)
{
    client_vec::iterator it;

    for (it = _client.begin(); it < _client.end(); ++it)
    {
        if (c_fd == (*it)->get_fd())
        {
            return (*it);
            break;
        }
    }
    return NULL;
}

Client* Server::getClient(std::string name)
{
    client_vec::iterator it;

    for (it = _client.begin(); it < _client.end(); ++it)
    {
        if (name == (*it)->get_nick())
        {
            return (*it);
            break;
        }
    }
    return NULL;
}

Client* Server::getClientByHost(std::string host)
{
    client_vec::iterator it;

    for (it = _client.begin(); it < _client.end(); ++it)
    {
        if (host == (*it)->get_host())
        {
            return (*it);
            break;
        }
    }
    return NULL;
}

Channel* Server::getChannel(std::string name)
{
    channel_vec::iterator it;

    for (it = _channel.begin(); it != _channel.end(); ++it)
    {
        if (name == (*it)->get_name())
        {
            return (*it);
            break;
        }
    }
    return NULL;
}

void Server::set_empty_string(int c_fd)
{
    _msg[c_fd] = "";
}

void Server::delClient(int c_fd)
{
    client_vec::iterator it;

    for (it = _client.begin(); it < _client.end(); ++it)
    {
        if((*it)->get_fd() == c_fd)
        {
            delete *it;
            _client.erase(it);
            break ;
        }
    }
    close(c_fd);
}

void Server::delChannel(Channel* channel)
{
    channel_vec::iterator it;

    for (it = _channel.begin(); it < _channel.end(); ++it)
    {
        if((*it)->get_name() == channel->get_name())
        {
            delete *it;
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
        if ((*it)->get_nick() == nick) 
            return (0);
    }
    return (1);
}

int Server::is_valid_channel(std::string name)
{
    channel_vec::iterator it;

    for (it = _channel.begin(); it < _channel.end(); ++it)
    {
        if ((*it)->get_name() == name) 
            return (0);
    }
    return (1);
}
