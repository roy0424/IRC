#include "../includes/Channel.hpp"

Channel::Channel(Client& client, std::string name, int flag) : _channel_name(name), _channel_mode(flag)
{
    _channel_opers.push_back(&client);
}

Channel::~Channel() 
{
}

std::string Channel::get_name() const
{
    return this->_channel_name;
}

std::string Channel::get_topic() const
{
    return this->_channel_topic;
}

int Channel::get_mode() const
{
    return this->_channel_mode;
}

std::string Channel::get_pwd() const
{
    return this->_pwd;
}

int	Channel::get_oper(int c_fd)
{
    client_vec::iterator it = _channel_opers.end();

    for (it = _channel_opers.begin(); it < _channel_opers.end(); ++it)
    {
        if (*it == NULL)
            return (1);
        if ((*it)->get_fd() == c_fd) 
        return (0);
    }
    return (1);
}

int	Channel::get_user(int c_fd)
{
    client_vec::iterator it = _channel_users.end();

    for (it = _channel_users.begin(); it < _channel_users.end(); ++it)
    {
        if (*it == NULL)
            return (1);
        if ((*it)->get_fd() == c_fd) // part -> get_user -> get-fd 여기 이런거에서 계속 세그터짐
            return (0);
    }
    return (1);
}

std::vector<Client*> Channel::get_users()
{
    return (this->_channel_users);
}

std::vector<Client*> Channel::get_opers()
{
    return (this->_channel_opers);
}

void Channel::set_topic(std::string topic)
{
    this->_channel_topic = topic;
}


int	Channel::check_invited(int c_fd)
{
    client_vec::iterator it = _invited_users.end();

    for (it = _invited_users.begin(); it < _invited_users.end(); ++it)
    {
        if ((*it)->get_fd() == c_fd) 
            return (0);
    }
    return (1);
}

void Channel::add_user(Client& client)
{
    _channel_users.push_back(&client);
}

void Channel::add_oper(Client& client)
{
    _channel_opers.push_back(&client);
}

void Channel::del_user(Client& client)
{
    client_vec::iterator it;

    if (_channel_users.size())
    {
        for (it = _channel_users.begin(); it < _channel_users.end(); ++it)
        {
            if ((*it)->get_fd() == client.get_fd())
            {
                _channel_users.erase(it);
                break;
            }
        }
    }
    del_oper(client);
}

void Channel::del_oper(Client& client)
{
    client_vec::iterator it;

    if (_channel_opers.size())
    {
        for (it = _channel_opers.begin(); it < _channel_opers.end(); ++it)
        {
            if ((*it)->get_fd() == client.get_fd())
            {
                _channel_opers.erase(it);
                break;
            }
        }
    }
}

void Channel::add_invit(Client& client)
{
    _invited_users.push_back(&client);
}

// mode
void Channel::set_mode(bool plus, int mode)
{
    _channel_mode = plus ? _channel_mode | mode : _channel_mode ^ mode;
}

void Channel::set_pwd(std::string pwd)
{
    _pwd = pwd;
}

void	Channel::set_maxmem(int num)
{
    _max = num;
}

int Channel::authorization(std::string c_name)
{
    client_vec::iterator it = _channel_users.end();

    for (it = _channel_users.begin(); it < _channel_users.end(); ++it)
    {
        if ((*it)->get_nick() == c_name)
        {
            if (get_oper((*it)->get_fd()))
                return (-1);
            add_oper(**it);
            del_user(**it);
            return (0);
        }
    }
    return (1);
}

int	Channel::deauthorization(std::string c_name)
{
    client_vec::iterator it = _channel_users.end();

    for (it = _channel_users.begin(); it < _channel_users.end(); ++it)
    {
        if ((*it)->get_nick() == c_name)
        {
            add_user(**it);
            del_oper(**it);
            return (0);
        }
    }
    return (1);
}


bool Channel::operator==(const void* other) const {
    return this == static_cast<const Channel*>(other);
}