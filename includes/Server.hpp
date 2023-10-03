#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <sys/socket.h>
#include <netinet/in.h> // sockaddr_in
#include <vector>
#include <iterator>
#include <poll.h>
#include <iostream>
#include <unistd.h> // close
#include <fcntl.h>
#include <cstring>
#include <exception>

#include "Client.hpp"
#include "Channel.hpp"
#include "Command.hpp"

# define BUF_SIZE 1024
const std::string SERVER_NAME = "irc.local";

class Server
{
	public:
		typedef std::vector<struct pollfd>	pollfd_vec;
		typedef std::vector<Client*>		client_vec;
		typedef std::vector<Channel*>		channel_vec;
		typedef std::vector<std::string>	str_vec;
	private:
		std::string			_pwd;
		int					_serv_socket_fd;
		struct sockaddr_in	_serv_addr;

		pollfd_vec			_pollfd_vec;
		client_vec			_client;
		channel_vec			_channel;

		char				_buf[BUF_SIZE];
		str_vec				_msg;
	public:
		Server(int port, std::string pwd);
		~Server();
		void push_back_fd(int fd, short event);
		void server_run();
		void accept_client();
		int	 recv_message(int fd);
		void send_message(std::string const& msg, int c_fd);
		void send_message(std::string const& cmd, std::string const& msg, Channel* channel, int c_fd);
		std::string get_message(int fd);
		str_vec split_commands(std::string msg);
		int	 execute_command(std::string msg, int c_fd);
		int compare_pwd(std::string const& cpwd);

		void handle_err(int err_num, std::string err_msg, int c_fd);

		void save_client(Client& client);
		void save_channel(Channel& channel);
		Client* getClient(int c_fd);
		Client* getClient(std::string name);
		Client* getClientByHost(std::string name);
		Channel* getChannel(std::string name);
		void set_empty_string(int c_fd);
		
		void	delClient(int c_fd);
		void	delChannel(Channel* channel);
		int is_valid_nick(int c_fd, std::string nick);
		int is_valid_channel(std::string name);
};

#endif
