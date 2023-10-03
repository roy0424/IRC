#include "../includes/Client.hpp"

Client::Client(int fd)
: _fd(fd)
{
}

Client::~Client() {}

int Client::get_fd() const
{
    return this->_fd;
}

std::string Client::get_nick() const
{
    return this->_nickname;
}

std::string Client::get_user() const
{
    return this->_username;
}

std::string Client::get_host() const
{
    return this->_hostname;
}

std::string Client::get_channel() const
{
    return this->_in_channel;
}

void Client::set_nick(std::string nick)
{
    _nickname = nick;
}

void Client::set_user(std::string user)
{
    _username = user;
}

void Client::set_host(std::string hostname)
{
    _hostname = hostname;
}

void Client::set_channel(std::string channel)
{
    _in_channel = channel;
}

bool Client::operator==(const void* other) const {
    return this == static_cast<const Client*>(other);
}