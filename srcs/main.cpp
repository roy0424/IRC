#include <sstream>
#include <cstdlib>
#include "../includes/Server.hpp"

int check_port(int argc, char *argv[])
{
	int port;

	if (argc != 3)
	{
		std::cout << "Usage : %s <port> <password>\n";
		return (-1);
	}
	port = std::atoi(argv[1]);
	std::stringstream ss;
    ss << port;
	if (port < 0 || port > 65535 || ss.str() != std::string(argv[1]))
	{
		std::cout << "Error : Wrong port num\n";
		return (-1);
	}
	return (port);
}

int main (int argc, char *argv[])
{
	int port = check_port(argc, argv);
	if (port == -1)
		return (1);
	try{
		Server server(port, argv[2]);
		server.server_run();
	} catch (std::exception& e) {
		std::cerr << e.what() << "\n";
		return (1);
	}
	return (0);
}
