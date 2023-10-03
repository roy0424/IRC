#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>

class Client
{
	public:
		typedef std::vector<std::string>	str_vec;

	private:
		int			_fd;
		std::string _nickname;
		std::string _username;
		std::string _hostname;
		str_vec		_channels;

	public:
		Client(int fd);
		~Client();

		int get_fd() const;
		std::string get_nick() const;
		std::string get_user() const;
		std::string get_host() const;
		std::vector<std::string> get_channel() const;
		void set_nick(std::string nick);
		void set_user(std::string user);
		void set_host(std::string hostname);
		void set_channel(std::string channel);
		void del_channel(std::string channel);

		bool operator==(const void* other) const;
};

#endif
