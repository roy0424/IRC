#include "../includes/Command.hpp"

Command::Command(std::string msg) : _msg(msg)
{
}

Command::~Command() 
{
}

void Command::split_command()
{    
    for (size_t pos = 0; pos != std::string::npos; )
    {
        pos = 0;
        pos = _msg.find(" ");
        _split_msg.push_back(_msg.substr(0, pos));
        if (_msg.length() <= 2)
            _msg = "";
        else
            _msg = _msg.substr(pos + 1, _msg.length());
    }
}

// topic 추가
int Command::classify_command(Server& server, int c_fd)
{
    split_command();
    std::string cmd = _split_msg[0];
    if (!cmd.compare("PASS"))
        cmd_pass(server, c_fd);
    else if(!cmd.compare("NICK"))
    {
        if (!cmd_nick(server, c_fd))
            return (0);
    }
    else if(!cmd.compare("USER"))
        cmd_user(server, c_fd);
    else if(!cmd.compare("PING"))
        cmd_ping(server, c_fd);
    else if(!cmd.compare("JOIN"))
        cmd_join(server, c_fd);
    else if(!cmd.compare("MODE"))
        cmd_mode(server, c_fd);
    else if(!cmd.compare("PART"))
        cmd_part(server, c_fd);
    else if(!cmd.compare("PRIVMSG"))
        cmd_privmsg(server, c_fd);
    else if(!cmd.compare("NOTICE"))
        cmd_notice(server, c_fd);
    else if(!cmd.compare("QUIT"))
        cmd_quit(server, c_fd);
    else if(!cmd.compare("KICK"))
        cmd_kick(server, c_fd);
    else if(!cmd.compare("INVITE"))
        cmd_invite(server, c_fd);
    else if(!cmd.compare("TOPIC"))
        cmd_topic(server, c_fd);
    else
        std::cout << "invalid CMD" << std::endl;
    return (1);
}

// ERR_ALREADYREGISTRED
void Command::cmd_pass(Server& server, int c_fd) 
{
    if (_split_msg.end() == _split_msg.begin())
        server.send_message(ERR_NEEDMOREPARAMS(server, c_fd), c_fd);
    if (server.compare_pwd(_split_msg[1]))
        server.send_message("Password is not correct\r\n", c_fd);
    else
    {
        Client client(c_fd);
        server.save_client(client);
    }
}
//ERR_NICKCOLLISION, ERR_NICKNAMEINUSE
int Command::cmd_nick(Server& server, int c_fd)
{
    std::string nick = _split_msg[1];
    Client& client = server.getClient(c_fd);

    if (client == NULL)
    {
        server.send_message(ERR_NOTREGISTERED(), c_fd);
        server.delClient(c_fd);
        return 0;
    }
    if (_split_msg.end() == _split_msg.begin())
    {
        server.send_message(ERR_NONICKNAMEGIVEN(server, c_fd), c_fd);
        return 1;
    }

    if (nick.length() > 9)
    {
        server.send_message(ERR_ERRONEUSNICKNAME(server, c_fd), c_fd);
        return 1;
    }
    if (nick.find(" ") != std::string::npos || nick.find(",") != std::string::npos
        || nick.find("*") != std::string::npos || nick.find("?") != std::string::npos 
        || nick.find("!") != std::string::npos || nick.find("@") != std::string::npos 
        || nick.find("$") != std::string::npos || nick.find(":") != std::string::npos 
        || nick.find("#") != std::string::npos || nick.find("&") != std::string::npos // channel types
        || nick.find(".") != std::string::npos)
    {
        server.send_message(ERR_ERRONEUSNICKNAME(server, c_fd), c_fd);
        return 1;
    }
    if (server.is_valid_nick(c_fd, nick))
    {
        server.send_message(":" + client.get_nick() + "!" + client.get_user() + "@" + client.get_host() + " NICK :" + nick + "\r\n", c_fd);
        client.set_nick(nick);
    }
    else
        server.send_message(ERR_NICKNAMEINUSE(server, c_fd), c_fd);
    return 1;
}

// C : <username> <hostname> <servername> <realname>
// S : :testnick USER guest tolmoon tolsun :Ronnie Reagan
void Command::cmd_user(Server& server, int c_fd)
{
    Client& client = server.getClient(c_fd);

    if (_split_msg.size() != 5)
    {
        server.send_message(ERR_NEEDMOREPARAMS(server, c_fd), c_fd);
        return ;
    }
    if (client.get_user() != "")
    {
        server.send_message(ERR_ALREADYREGISTRED(server, c_fd), c_fd);
        return ;
    }
    client.set_user(_split_msg[1]);
    client.set_host(_split_msg[3]);
    server.send_message(":" + client.get_nick() + " " + _split_msg[0] + " " + client.get_user() + " " + client.get_host() + " " + _split_msg[3] + " " + _split_msg[4] + "\r\n", c_fd);
    server.send_message(RPL_WELCOME(server, c_fd), c_fd);
    server.send_message(RPL_YOURHOST(server, c_fd), c_fd);
}

// C : PING :irc.local
// S : :irc.local PONG irc.local :irc.local
//ERR_NOORIGIN, ERR_NOSUCHSERVER
void Command::cmd_ping(Server& server, int c_fd)
{
    server.send_message(":" + SERVER_NAME + " PONG " + SERVER_NAME + " :" + SERVER_NAME + "\r\n", c_fd);
}

// C : JOIN #hi []
// S : :test!ubuntu@127.0.0.1 JOIN :#hi
// S : :irc.local 353 test = #hi :@test test2 (전체 user)
// S : :irc.local 366 test #hi :End of /NAMES list.
// ERR_BANNEDFROMCHAN, ERR_CHANNELISFULL, ERR_BADCHANMASK, ERR_NOSUCHCHANNEL, ERR_TOOMANYCHANNELS
void Command::cmd_join(Server& server, int c_fd)
{
    std::string name = _split_msg[1];
    Client& client = server.getClient(c_fd);
    std::string send_message = "";

    if (_split_msg.size() < 2)
    {
        server.send_message(ERR_NEEDMOREPARAMS(server, c_fd), c_fd);
        return ;
    }

    //성공
    if (server.is_valid_channel(name))
    {
        Channel channel(client, name, T);
        client.set_channel(name);
        server.save_channel(channel);
        send_message += ":" + client.get_nick() + "!" + client.get_user() + "@" + client.get_host() + " " + _split_msg[0] + " :" + name + "\n";
    }
    else
    {
        Channel& chan = server.getChannel(name);
        if (chan.get_mode() & K || chan.get_mode() & (I | K))
        {
            std::string pwd;
            if (_split_msg.size() != 3 || pwd.compare(chan.get_pwd()))
            {
                server.send_message(ERR_BADCHANNELKEY(server, c_fd), c_fd);
                return ;
            }
        }
        if (chan.get_mode() & I || chan.get_mode() & (I | K))
        {
            if (chan.check_invited(c_fd))
            {
                server.send_message(ERR_INVITEONLYCHAN(server, c_fd), c_fd);
                return ;
            }
        }
        chan.add_user(client);
        client.set_channel(name);
        send_message += ":" + client.get_nick() + "!" + client.get_user() + "@" + client.get_host() + " " + _split_msg[0] + " :" + name + "\n";
        if (chan.get_topic() != "") // 332 topic
            send_message += RPL_TOPIC(server, c_fd, chan) + "\r\n";
    }
    // 353 user list
    send_message += RPL_NAMREPLY(server, c_fd, server.getChannel(name)) + "\r\n";
    // 366 ban list
    send_message += RPL_ENDOFNAMES(server, c_fd, server.getChannel(name)) + "\r\n";
    server.send_message(send_message, c_fd);
    // 전체 메세지
    server.send_message("JOIN", "", server.getChannel(name), c_fd); // 여기도 터짐
}

// C : MODE #hi -i+k 1234
// S : :test!ubuntu@127.0.0.1 MODE #hi :+i
//ERR_NOTONCHANNEL, ERR_KEYSET, RPL_BANLIST, RPL_ENDOFBANLIST, ERR_USERSDONTMATCH, RPL_UMODEIS, ERR_UMODEUNKNOWNFLAG
void Command::cmd_mode(Server& server, int c_fd)
{
    Channel& channel = server.getChannel(_split_msg[1]);
    Client& client = server.getClient(c_fd);
    if (channel == NULL || _split_msg[1][0] != '#')
        return ;
    std::string name(_split_msg[1]);

    std::cout << "mode msg size : " << _split_msg.size() << std::endl;
    if (_split_msg.size() < 2)
    {
        server.send_message(ERR_NEEDMOREPARAMS(server, c_fd), c_fd);
        return ;
    }
    if (_split_msg.size() == 2)
    {
        if (!name.compare(channel.get_name()))
            server.send_message(RPL_CHANNELMODEIS(server, c_fd, channel), c_fd);
        else
            server.send_message(ERR_NOSUCHCHANNEL(server, c_fd), c_fd);
        return ;
    }
    if (channel.get_oper(c_fd))
    {
        server.send_message(ERR_CHANOPRIVSNEEDED(server, c_fd, channel.get_name()), c_fd);
        return ;
    }

    std::string mode = _split_msg[2];
    size_t idx = 3;
    bool plus;
    int ex_mode = channel.get_mode();
    int curr_mode;
    int oper;

    for (size_t i = 0; i < mode.length(); ++i)
    {
        if (!i && mode[i] != '+' && mode[i] != '-')
            break ;
        if (mode[i] == '+')
            plus = true;
        else if (mode[i] == '-')
            plus = false;
        else if (mode[i] == 't')
            channel.set_mode(plus, T);
        else if (mode[i] == 'i')
            channel.set_mode(plus, I);
        else if (mode[i] == 'k')
        {
            if (_split_msg.size() >= idx)
            {
                if (plus)
                    channel.set_pwd(_split_msg[idx++]);
                else
                    channel.set_pwd(NULL);
                channel.set_mode(plus, K);
            }
            else
                server.send_message(ERR_NEEDMOREPARAMS(server, c_fd), c_fd);
        }
        else if (mode[i] == 'o')
        {
            if (_split_msg.size() >= idx)
            {
                if (plus)
                {
                    oper = channel.authorization(_split_msg[idx++]);
                    if (oper == 1)
                        server.send_message(ERR_NOSUCHNICK(server, c_fd), c_fd);
                }
                else
                {
                    oper = channel.deauthorization(_split_msg[idx++]);
                    if (oper)
                        server.send_message(ERR_NOSUCHNICK(server, c_fd), c_fd);
                }
            }
            else
                server.send_message(ERR_NEEDMOREPARAMS(server, c_fd), c_fd);
        }
        else if (mode[i] == 'l')
        {
            if (_split_msg.size() >= idx)
            {
                if (plus)
                    channel.set_maxmem(std::atoi(_split_msg[idx++].c_str()));
                else
                    channel.set_maxmem(0);
                channel.set_mode(plus, L);
            }
            else
                server.send_message(ERR_NEEDMOREPARAMS(server, c_fd), c_fd);
        }
        else
        {
            server.send_message(ERR_UNKNOWNMODE(server, c_fd, mode[i]), c_fd);
            return ;
        }
    } 
    
    // :test!ubuntu@127.0.0.1 MODE #hi +kit :wefwwe 모든 클라이언트
    // C : MODE #hi +kol-t 1234 test2 10
    // S : :test!ubuntu@127.0.0.1 MODE #hi +kl-t 1234 :10 // 마지막 거에만 : 붙음..
    curr_mode = channel.get_mode();
    std::string plus_mode = mode.find('+') != std::string::npos ? "+" : "";
    std::string minus_mode = mode.find('-') != std::string::npos ? "-" : "";
    std::string m_mode;
    idx = mode.find('-');
    m_mode = idx != std::string::npos ? mode.substr(idx) : "";
    for (size_t i = 0; i < mode.length(); ++i)
    {
        if (mode[i] == '+')
            plus = true;
        else if (mode[i] == '-')
            plus = false;
        else if (mode[i] == 't' && (ex_mode ^ curr_mode) & T)
            plus ? plus_mode.push_back('t') : minus_mode.push_back('t');
        else if (mode[i] == 'k' && (ex_mode ^ curr_mode) & K)
            plus ? plus_mode.push_back('k') : minus_mode.push_back('k');
        else if (mode[i] == 'i' && (ex_mode ^ curr_mode) & I)
            plus ? plus_mode.push_back('i') : minus_mode.push_back('i');
        else if (mode[i] == 'l' && (ex_mode ^ curr_mode) & L)
            plus ? plus_mode.push_back('l') : minus_mode.push_back('l');
        else if (mode[i] == 'o' && !oper)
            plus ? plus_mode.push_back('o') : minus_mode.push_back('o');
    }
    plus_mode += minus_mode;
    for (idx = 3; idx < _split_msg.size() - 1; ++idx)
        plus_mode += " " + _split_msg[idx];
    if (_split_msg.size() > 3)
        plus_mode += " :" + _split_msg[idx];
    server.send_message(":" + client.get_nick() + "!" + client.get_user() + "@" + client.get_host() + " " + _split_msg[0] + " " + name + " " + plus_mode + "\r\n", c_fd);
    server.send_message("MODE", plus_mode, channel, c_fd);
}



// C : PART #hi
// S : :test3!ubuntu@127.0.0.1 PART :#hi // 전체 전송, channel 에 아무도없으면 채널 삭제
void Command::cmd_part(Server& server, int c_fd)
{
    std::string channel_name;
    
    if (_split_msg.size() < 2)
    {
        server.send_message(ERR_NEEDMOREPARAMS(server, c_fd), c_fd);
        return ;
    }
    channel_name = _split_msg[1];
    Channel& channel = server.getChannel(channel_name);
    if (channel == 0)
    {
        server.send_message(ERR_NOSUCHCHANNEL(server, c_fd), c_fd);
        return ;
    }
    if (channel.get_oper(c_fd) && channel.get_user(c_fd))
        return ;
    Client& client = server.getClient(c_fd);
    if (client == 0)
    {
        server.send_message(ERR_NOTONCHANNEL(server, c_fd), c_fd);
        return ;
    }
    channel.del_user(client);
    client.set_channel("");
    if (!channel.get_users().size() && !channel.get_opers().size())
    {
        server.delChannel(channel);
        server.send_message(":" + client.get_nick() + "!" + client.get_user() + "@" + client.get_host() + " PART :" + channel_name + "\r\n", c_fd);
    }
    else
    {
        server.send_message(":" + client.get_nick() + "!" + client.get_user() + "@" + client.get_host() + " PART :" + channel_name + "\r\n", c_fd);
        server.send_message("PART", ":" + channel_name, channel, c_fd);
    }
}

// C : PRIVMSG #hi :hihihi
// S : test3!ubuntu@127.0.0.1 PRIVMSG #hi :hihihi  // 체널 안 전체 전송
// C : PRIVMSG test2 :hihi
// S : :test!ubuntu@127.0.0.1 PRIVMSG test2 :hihi  // 한 명한테 전송
void Command::cmd_privmsg(Server& server, int c_fd)
{
    std::string target;
    std::string msg;
    std::vector<std::string>::iterator it;

    if (_split_msg.size() < 2)
    {
        server.send_message(ERR_NORECIPIENT(server, c_fd, "PRIVMSG"), c_fd);
        return ;
    }
    if (_split_msg.size() < 3)
    {
        server.send_message(ERR_NOTEXTTOSEND(server, c_fd), c_fd);
        return ;
    }
    target = _split_msg[1];
    for (it = _split_msg.begin() + 2; it < _split_msg.end(); ++it)
        msg += *it + " ";
    msg.erase(msg.length() - 1);
    if (target[0] == '#')
    {
        Channel &channel = server.getChannel(target);
        server.send_message("PRIVMSG", msg, channel, c_fd);
    }
    else
    {
        Client &client = server.getClient(target);
        if (client == 0)
        {
            server.send_message(ERR_NOSUCHNICK(server, c_fd), c_fd);
            return ;
        }
        server.send_message(":" + client.get_nick() + "!" + client.get_user() + "@" + client.get_host() + " PRIVMSG " + client.get_nick() + " " + msg + "\r\n", c_fd);
    }
}

// 평가 항목에 있음
// NOTICE 메시지는 PRIVMSG와 유사하게 사용됩니다. 
// NOTICE 와 PRIVMSG의 차이점은 NOTICE 메시지에 대한 응답으로 자동 응답을 전송해서는 안 된다는 것입니다.
// 서버는 통지를 수신할 때 클라이언트에 오류 회신을 다시 보내지 않아야 합니다. 
// 이 규칙의 목적은 클라이언트가 수신한 것에 대한 응답으로 자동으로 무언가를 보내는 사이의 루프를 방지하는 것입니다. 
// 이것은 일반적으로 자동화 장치(AI 또는 다른 대화형 프로그램이 동작을 제어하는 클라이언트)에서 사용되며, 항상 다른 자동화 장치와 루프 상태가 되지 않도록 응답하는 것으로 보입니다. 
// S : :irc.local NOTICE test :*** Could not resolve your hostname: Request timed out; using your IP address (127.0.0.1) instead.
// :irc.local 001 test :Welcome to the Localnet IRC Network test!ubuntu@127.0.0.1
void Command::cmd_notice(Server& server, int c_fd)
{
    (void)server;
    (void)c_fd;
}

// C : QUIT :leaving
// S : ERROR :Closing link: (ubuntu@127.0.0.1) [Quit: leaving]
void Command::cmd_quit(Server& server, int c_fd)
{
    std::string msg;
    std::vector<std::string>::iterator it;
    Client& client = server.getClient(c_fd);

    if (_split_msg.size() > 2)
    {
        for (it = _split_msg.begin() + 3; it < _split_msg.end(); ++it)
            msg += *it + " ";
        msg.erase(msg.length() - 1);
    }
    else
        msg = "leaving";
    server.send_message("ERROR :Closing link: (" + client.get_user() + "@" + client.get_host() + ") [Quit: " + msg + "]\r\n", c_fd);
    if (client.get_channel() != "")
        server.getChannel(client.get_channel()).del_user(client);
    server.delClient(c_fd);
}

// C : KICK #hi test :you are babo
// S : :test2!ubuntu@127.0.0.1 KICK #hi test :you are babo    전부한테
//ERR_BADCHANMASK
void Command::cmd_kick(Server& server, int c_fd)
{
    std::string channel_name;
    std::string client_name;
    std::string reason;
    std::vector<std::string>::iterator it;

    if (_split_msg.size() < 4)
    {
        server.send_message(ERR_NEEDMOREPARAMS(server, c_fd), c_fd);
        return ;
    }
    channel_name = _split_msg[1];
    client_name = _split_msg[2];
    for (it = _split_msg.begin() + 3; it < _split_msg.end(); ++it)
        reason += *it + " ";
    reason.erase(reason.length() - 1);
    Client &client = server.getClient(client_name);
    Channel &channel = server.getChannel(channel_name);
    if (client == 0 || channel == 0)
    {
        server.send_message(ERR_NOSUCHNICK(server, c_fd), c_fd);
        return ;
    }
    if (channel.get_user(client.get_fd()) == 0)
    {
        server.send_message(ERR_NOTONCHANNEL(server, c_fd), c_fd);
        return ;
    }
    if (channel.get_oper(c_fd))
    {
        server.send_message(ERR_CHANOPRIVSNEEDED(server, c_fd, channel_name), c_fd);
        return ;
    }
    // 자신한테도?
    server.send_message("KICK", client_name + " :" + reason, channel, c_fd);
}

// C : INVITE test #hi                              test 를 초대 
// S : :irc.local 341 test2 test :#hi               test2한테
// S : :test2!ubuntu@127.0.0.1 INVITE test :#hi     test 한테
//RPL_INVITING, RPL_AWAY
void Command::cmd_invite(Server& server, int c_fd)
{
    std::string client_name;
    std::string channel_name;
    Client& hclient = server.getClient(c_fd);

    if (_split_msg.size() < 3)
    {
        server.send_message(ERR_NEEDMOREPARAMS(server, c_fd), c_fd);
        return ;
    }
    client_name = _split_msg[1];
    channel_name = _split_msg[2];
    Client& client = server.getClient(client_name);
    Channel& channel = server.getChannel(channel_name);
    if (client == NULL || channel == NULL)
    {
        server.send_message(ERR_NOSUCHNICK(server, c_fd), c_fd);
        return ;
    }
    if (!channel.get_user(c_fd) && !channel.get_oper(c_fd))
    {
        server.send_message(ERR_NOTONCHANNEL(server, c_fd), c_fd);
        return ;
    }
    if (channel.get_user(client.get_fd())) // get_fd 에러에러에러!!!!!!
    {
        server.send_message(ERR_USERONCHANNEL(server, c_fd, channel_name), c_fd);
        return ;
    }
    if (!channel.get_oper(c_fd))
    {
        server.send_message(ERR_CHANOPRIVSNEEDED(server, c_fd, channel_name), c_fd);
        return ;
    }
    if (!server.is_client(client.get_fd()))
    {
        server.send_message(ERR_NOSUCHNICK(server, c_fd), c_fd);
        return ;
    }
    channel.add_invit(client);
    server.send_message(":" + SERVER_NAME + " 341 " + client.get_nick() + " " + client_name + " :" + channel_name + "\r\n", c_fd);
    server.send_message(":" + hclient.get_nick() + "!" + hclient.get_user() + "@" + hclient.get_host() + " INVITE " + client_name + " :" + channel_name + "\r\n", client.get_fd());
}
// :irc.local 461 test USERHOST :Not enough parameters.
// :irc.local 432 test weofijweiofjweijf*** :Erroneous Nickname \r\n

// C : TOPIC #hi :hihi
// S :test!root@127.0.0.1 TOPIC #hi :hihi
void Command::cmd_topic(Server& server, int c_fd)
{
    Channel& channel = server.getChannel(_split_msg[1]);
    std::vector<std::string>::iterator it;
    std::string msg;

    if (_split_msg.size() == 1)
    {
        server.send_message(ERR_NEEDMOREPARAMS(server, c_fd), c_fd);
        return;
    }
    if (channel == NULL)
        return ;
    if (_split_msg.size() == 2) // 461
    {
        if (channel.get_topic() == "")
            server.send_message(RPL_NOTOPIC(server, c_fd, channel), c_fd);
        else
            server.send_message(RPL_TOPIC(server, c_fd, channel), c_fd);
        return ;
    }
    else
    {
        if (channel.get_user(c_fd) && channel.get_oper(c_fd))
        {
            server.send_message(ERR_NOTONCHANNEL(server, c_fd), c_fd);
        }
        else if(channel.get_oper(c_fd))
        {
            server.send_message(ERR_CHANOPRIVSNEEDED(server, c_fd, channel.get_name()), c_fd);
        }
        else
        {
            for (it = _split_msg.begin() + 2; it < _split_msg.end(); ++it)
            msg += *it + " ";
            msg.erase(msg.length() - 1);
            channel.set_topic(msg);
            server.send_message(RPL_TOPIC(server, c_fd, channel), c_fd);
            server.send_message("TOPIC", " :" + msg, channel, c_fd);
        }
    }
}

const std::string Command::ERR_NEEDMOREPARAMS(Server& server, int c_fd)
{
    return (":" + SERVER_NAME + " 461 " + server.getClient(c_fd).get_nick() + " " + _split_msg[0] + " :Not enough parameters\r\n");
}

const std::string Command::ERR_ALREADYREGISTRED(Server& server, int c_fd)
{
    return (":" + SERVER_NAME + " 462 " + server.getClient(c_fd).get_nick() + " :You may not register\r\n");
}

const std::string Command::ERR_NONICKNAMEGIVEN(Server& server, int c_fd)
{
    return (":" + SERVER_NAME + " 431 " + server.getClient(c_fd).get_nick() + " :No nickname given\r\n");
}		

const std::string Command::ERR_ERRONEUSNICKNAME(Server& server, int c_fd)
{
    return (":" + SERVER_NAME + " 432 " + server.getClient(c_fd).get_nick() + " " + _split_msg[1] + " :Erroneous Nickname\r\n");
}

const std::string Command::ERR_NICKNAMEINUSE(Server& server, int c_fd)
{
    return (":" + SERVER_NAME + " 433 " + server.getClient(c_fd).get_nick() + " " + _split_msg[1] + " :Nickname is already in use\r\n");
}

const std::string Command::ERR_NICKCOLLISION(Server& server, int c_fd)
{
    return (":" + SERVER_NAME + " 436 " + server.getClient(c_fd).get_nick() + " " + _split_msg[1] + " :Nickname collision KILL\r\n");
}

const std::string Command::ERR_NOORIGIN(Server& server, int c_fd)
{
    return (":" + SERVER_NAME + " 409 " + server.getClient(c_fd).get_nick() + " :No origin specified\r\n");
}

const std::string Command::ERR_NOSUCHSERVER(Server& server, int c_fd)
{
    return (":" + SERVER_NAME + " 402 " + server.getClient(c_fd).get_nick() + " " + _split_msg[1] + " :No such server\r\n");
}

const std::string Command::ERR_BANNEDFROMCHAN(Server& server, int c_fd)
{
    return (":" + SERVER_NAME + " 474 " + server.getClient(c_fd).get_nick() + " " + _split_msg[1] + " :Cannot join channel (+b)\r\n");
}

const std::string Command::ERR_INVITEONLYCHAN(Server& server, int c_fd)
{
    return (":" + SERVER_NAME + " 473 " + server.getClient(c_fd).get_nick() + " " +_split_msg[1] + " :Cannot join channel (+i)\r\n");
}

const std::string Command::ERR_BADCHANNELKEY(Server& server, int c_fd)
{
    return (":" + SERVER_NAME + " 475 " + server.getClient(c_fd).get_nick() + " " + _split_msg[1] + " :Cannot join channel (+k)\r\n");
}


const std::string Command::ERR_CHANNELISFULL(Server& server, int c_fd)
{
    return (":" + SERVER_NAME + " 471 " + server.getClient(c_fd).get_nick() + " " + " :Cannot join channel (+l)\r\n");
}

const std::string Command::ERR_BADCHANMASK(Server& server, int c_fd)
{
    return (":" + SERVER_NAME + " 476 " +server.getClient(c_fd).get_nick() + " " + " :Cannot join channel (+i) - You're not invited\r\n");
}

const std::string Command::ERR_NOSUCHCHANNEL(Server& server, int c_fd)
{
    return (":" + SERVER_NAME + " 403 "+ server.getClient(c_fd).get_nick() + " "  + _split_msg[1] + " :No such channel\r\n");
}

const std::string Command::ERR_TOOMANYCHANNELS(Server& server, int c_fd)
{
    return (":" + SERVER_NAME + " 405 "+ server.getClient(c_fd).get_nick() + " "  + _split_msg[1] + " :You have joined too many channels\r\n");
}


const std::string Command::ERR_NOTONCHANNEL(Server& server, int c_fd)
{
    return (":" + SERVER_NAME + " 442 "+ server.getClient(c_fd).get_nick() + " "  + _split_msg[1] + "\r\n");
}

const std::string Command::ERR_USERONCHANNEL(Server& server, int c_fd, std::string ch_name)
{
    return (":" + SERVER_NAME + " 443 "+ server.getClient(c_fd).get_nick() + " "  + _split_msg[1] + " " + ch_name + " :is already on channel\r\n");
}

const std::string Command::ERR_NOTREGISTERED()
{   //:서버이름 451 닉네임 :You have not registered
    return (":" + SERVER_NAME + " 451 " + _split_msg[1] + " :You have not registered\r\n");
}

const std::string Command::ERR_NORECIPIENT(Server& server, int c_fd, std::string cmd)
{
    return (":" + SERVER_NAME + " 411 " + server.getClient(c_fd).get_nick() + " :No recipient given " + cmd +"\r\n");

}

const std::string Command::ERR_NOTEXTTOSEND(Server& server, int c_fd)
{
    return (":" + SERVER_NAME + " 412 " + server.getClient(c_fd).get_nick() + " :No text to send\r\n");

}

const std::string Command::ERR_CHANOPRIVSNEEDED(Server& server, int c_fd, std::string ch_name)
{
    return (":" + SERVER_NAME + " 482 " + server.getClient(c_fd).get_nick() + " " + ch_name + " :You're not channel operator\r\n");
}

const std::string Command::ERR_NOSUCHNICK(Server& server, int c_fd)
{
    return (":" + SERVER_NAME + " 401 " + server.getClient(c_fd).get_nick() + " :No such nick/channel\r\n");
}

const std::string Command::ERR_USERSDONTMATCH(Server& server, int c_fd)
{
    return (":" + SERVER_NAME + " 502 " + server.getClient(c_fd).get_nick() + " :Cannot change mode for other users\r\n");
}

const std::string Command::ERR_UNKNOWNMODE(Server&server, int c_fd, char mode)
{
    return (":" + SERVER_NAME + " 472 " + server.getClient(c_fd).get_nick() + mode + " :is unknown mode char to me\r\n");
}

// "Welcome to the Internet Relay Network <nick>!<user>@<host>"
const std::string Command::RPL_WELCOME(Server& server, int c_fd)
{
    return (":" + SERVER_NAME + " 001 " + server.getClient(c_fd).get_nick() + " Welcome to the Internet Relay Network " + server.getClient(c_fd).get_nick() + "!" + server.getClient(c_fd).get_user() + "@" + server.getClient(c_fd).get_host() + "\r\n");
}

// "Your host is <servername>, running version <ver>"
const std::string Command::RPL_YOURHOST(Server& server, int c_fd)
{
    return (":" + SERVER_NAME + " 002 " + server.getClient(c_fd).get_nick() + " Your host is " + SERVER_NAME + ", running version 1.0.1\r\n");
}

//ex) "<channel> :No topic is set" 
const std::string Command::RPL_NOTOPIC(Server& server, int c_fd, Channel& channel) // 331
{
    return (":" + SERVER_NAME + " 331 " + server.getClient(c_fd).get_nick() + " " + channel.get_name() + " :No topic is set\r\n");

}

//ex) <서버 이름> 332 <본인 닉네임> <채널 이름> :<토픽>
const std::string Command::RPL_TOPIC(Server& server, int c_fd, Channel& channel)
{
    return (":" + SERVER_NAME + " 332 " + server.getClient(c_fd).get_nick() + " " + channel.get_name() + " :" + channel.get_topic() + "\r\n");
}

const std::string Command::RPL_NAMREPLY(Server& server, int c_fd, Channel& channel) // 353
{
    std::string nameList = "";
    std::vector<Client*> opers = channel.get_opers();
    std::vector<Client*> users = channel.get_users();

    for (std::vector<Client*>::iterator it = opers.begin(); it < opers.end(); ++it)
        nameList += "@" + (*it)->get_nick() + " ";
    for (std::vector<Client*>::iterator it = users.begin(); it < users.end(); ++it)
        nameList += (*it)->get_nick() + " ";
    return (":" + SERVER_NAME + " 353 " + server.getClient(c_fd).get_nick() + " = " + channel.get_name() + " :" + nameList);

}

const std::string Command::RPL_ENDOFNAMES(Server& server, int c_fd, Channel& channel) // 366
{
    return (":" + SERVER_NAME + " 366 " + server.getClient(c_fd).get_nick() + " " + channel.get_name() + " :End of /WHO list.\r\n");
}




// const std::string Command::RPL_UMODEIS(Server& server, int c_fd) //221
// {
    
// }

const std::string Command::RPL_CHANNELMODEIS(Server& server, int c_fd, Channel &channel)
{
    std::string mode = "";
    if (channel.get_mode() & (T | K | I))
        mode += "+";
    if (channel.get_mode() & I)
        mode += "i";
    if (channel.get_mode() & K)
        mode += "k";
    if (channel.get_mode() & T)
        mode += "t";
    return (":" + SERVER_NAME + " 324 " + server.getClient(c_fd).get_nick() + " " + channel.get_name() + " " + mode + " :" + channel.get_pwd()) + "\r\n";
} 


void Command::print()
{
    for (std::vector<std::string>::iterator it = _split_msg.begin(); it != _split_msg.end(); it++)
    {
        std::cout << *it << std::endl;
    }
}

// int main() {
//     Command cmd("abc def ghi");
//     cmd.split_command();
//     cmd.print();
// }
