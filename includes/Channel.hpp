#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include <sys/socket.h>

//#include <iostream>

#include "Client.hpp"

enum Flag {
	NONE = 1 << 0,
	I = 1 << 1,
	K = 1 << 2,
	T = 1 << 3,
	L = 1 << 4
};

class Channel {
	public :
		typedef std::vector<Client*> client_vec; // 포인터로 할 경우 나중에 delete 잘해줘야함
	private :
		std::string _pwd;
		std::string _channel_name; 
		std::string _channel_topic;
		client_vec  _channel_users;
		client_vec  _channel_opers;
		client_vec	_invited_users;
		int _channel_mode;
		int _max;

	public :
		Channel(Client& client, std::string name, int flag);
		~Channel();

		std::string get_name() const;
		std::string get_topic() const;
		int 		get_mode() const;
		std::string get_pwd() const;
		int			get_oper(int c_fd);
		int			get_user(int c_fd);
		std::vector<Client*>	get_users();
		std::vector<Client*>	get_opers();

		void	set_topic(std::string topic); //
		int		check_invited(int c_fd);
		void	add_user(Client& client);
		void 	add_oper(Client& client);
		void	del_user(Client& client);
		void 	del_oper(Client& client);
		void 	add_invit(Client& client);
		void	send_message(std::string const& cmd, std::string const& msg);

		// mode
		void	set_mode(bool plus, int mode);
		void 	set_pwd(std::string pwd);
		void	set_maxmem(int num);
		int		authorization(std::string c_name);
		int		deauthorization(std::string c_name);

		bool operator==(const void* other) const;

};

#endif
