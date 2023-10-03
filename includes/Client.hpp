#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
	private:
		int			_fd;
		// int			_flag;
		std::string _nickname;
		std::string _username;
		std::string _hostname;
		std::string _in_channel;

	public:
		Client(int fd);
		~Client();

		int get_fd() const;
		std::string get_nick() const;
		std::string get_user() const;
		std::string get_host() const;
		std::string get_channel() const;
		void set_nick(std::string nick);
		void set_user(std::string user);
		void set_host(std::string hostname);
		void set_channel(std::string channel);

		bool operator==(const void* other) const;
};

#endif
