#ifndef COMMAND_HPP
#define COMMAND_HPP

#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include <sstream>
#include <iostream>
#include <vector>
#include <cstdlib>

class Server;

class Command {
	private :
		std::string					_msg;
		std::vector<std::string>	_split_msg;

	public :
		Command(std::string msg);
		~Command();
		int	 classify_command(Server& server, int c_fd);
		void split_command();
		void print();

        // command list
        // pass nick user ping join part privmsg notice quit oper kick invite
		void cmd_pass(Server& server, int c_fd);
        int	 cmd_nick(Server& server, int c_fd);
        void cmd_user(Server& server, int c_fd);
		void cmd_ping(Server& server, int c_fd);
		void cmd_join(Server& server, int c_fd);
		void cmd_mode(Server& server, int c_fd);
		void cmd_part(Server& server, int c_fd);
		void cmd_privmsg(Server& server, int c_fd);
		void cmd_notice(Server& server, int c_fd); //
		void cmd_quit(Server& server, int c_fd);
		void cmd_kick(Server& server, int c_fd); //
		void cmd_invite(Server& server, int c_fd); //
		void cmd_topic(Server& server, int c_fd);
		void cmd_oper(Server& server, int c_fd); //

		const std::string ERR_NEEDMOREPARAMS(Server& server, int c_fd); // 461
		const std::string ERR_ALREADYREGISTRED(Server& server, int c_fd); // 462
		const std::string ERR_NONICKNAMEGIVEN(Server& server, int c_fd); // 431
		const std::string ERR_ERRONEUSNICKNAME(Server& server, int c_fd); // 432
		const std::string ERR_NICKNAMEINUSE(Server& server, int c_fd); // 433
		const std::string ERR_NICKCOLLISION(Server& server, int c_fd); // 434
		const std::string ERR_NOORIGIN(Server& server, int c_fd); // 409
		const std::string ERR_NOSUCHSERVER(Server& server, int c_fd); // 402
		const std::string ERR_BANNEDFROMCHAN(Server& server, int c_fd); // 474
		const std::string ERR_INVITEONLYCHAN(Server& server, int c_fd); // 473
		const std::string ERR_BADCHANNELKEY(Server& server, int c_fd); // 475
		const std::string ERR_CHANNELISFULL(Server& server, int c_fd); // 471
		const std::string ERR_BADCHANMASK(Server& server, int c_fd); // 476
		const std::string ERR_NOSUCHCHANNEL(Server& server, int c_fd); // 403
		const std::string ERR_TOOMANYCHANNELS(Server& server, int c_fd); // 405
		const std::string ERR_NOTONCHANNEL(Server& server, int c_fd); // 442
		const std::string ERR_USERONCHANNEL(Server& server, int c_fd, std::string ch_name); // 443
		const std::string ERR_NOTREGISTERED(); // 451
		const std::string ERR_NORECIPIENT(Server& server, int c_fd, std::string cmd); // 411
		const std::string ERR_NOTEXTTOSEND(Server& server, int c_fd); // 412
		const std::string ERR_CHANOPRIVSNEEDED(Server& server, int c_fd, std::string ch_name); //482
		const std::string ERR_NOSUCHNICK(Server& server, int c_fd); // 401
		const std::string ERR_USERSDONTMATCH(Server& server, int c_fd); // 502
		const std::string ERR_UNKNOWNMODE(Server&server, int c_fd, char mode); // 472


		const std::string RPL_WELCOME(Server& server, int c_fd); // 001
		const std::string RPL_YOURHOST(Server& server, int c_fd); // 002

		// join
		const std::string RPL_TOPIC(Server& server, int c_fd, Channel& channel); // 332
		const std::string RPL_NAMREPLY(Server& server, int c_fd, Channel& channel); // 353
		const std::string RPL_ENDOFNAMES(Server& server, int c_fd, Channel& channel); // 366
		

		const std::string RPL_CHANNELMODEIS(Server& server, int c_fd, Channel& channel); // 324
		//const std::string RPL_UMODEIS(Server& server, int c_fd); // 221
		
		//topic
		const std::string RPL_NOTOPIC(Server& server, int c_fd, Channel& channel); // 331


};

#endif
