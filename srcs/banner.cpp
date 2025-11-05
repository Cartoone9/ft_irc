#include "utils.hpp"

void displayBanner(int port, State &state)
{
	const int width = 50;
	const int height = 7;

	std::string banner[] = {
		"  #### ########   ######  ",
		"   ##  ##     ## ##    ## ",
		"   ##  ##     ## ##       ",
		"   ##  ########  ##       ",
		"   ##  ##   ##   ##       ",
		"   ##  ##    ##  ##    ## ",
		"  #### ##     ##  ######  "
	};

	std::cout << std::endl;

	// Top border
	std::cout << "+" << std::string(width - 2, '-') << "+" << std::endl;

	// Empty padding line
	std::cout << "|" << std::string(width - 2, ' ') << "|" << std::endl;

	// Print banner centered
	for (int i = 0; i < height; ++i)
	{
		int padding = (width - 2 - banner[i].size()) / 2;
		std::cout << "|" 
			<< std::string(padding, ' ') 
			<< banner[i] 
			<< std::string(width - 2 - padding - banner[i].size(), ' ') 
			<< "|" << std::endl;
	}

	// Empty padding line & border
	std::cout << "|" << std::string(width - 2, ' ') << "|" << std::endl;
	std::cout << "+" << std::string(width - 2, '-') << "+" << std::endl;

	// Ip & port number
	std::ostringstream	port_oss;
	port_oss << port;
	std::string	port_str = "Port: " + port_oss.str();
	std::string ip_str = "Ip: 0.0.0.0";

	bool display_pass = true;
	int space_total = width - 2 - ip_str.size() - state.password.size() - port_str.size(); // -2 for "|" chars
	if (space_total <= 0)
	{
		space_total = width - 2 - ip_str.size() - port_str.size();
		display_pass = false;
	}

	int space_left = space_total / 2;
	int space_right = space_total - space_left; // handle odd spaces

	std::cout << "|" 
		<< ip_str
		<< std::string(space_left, ' ')
		<< (display_pass ? state.password : "")
		<< std::string(space_right, ' ')
		<< port_str
		<< "|" << std::endl;

	// Bottom border
	std::cout << "+" << std::string(width - 2, '-') << "+" << std::endl;

	std::cout << std::endl;

	// MOTD
	std::cout << UNDERLINE "Message Of The Day:" RESET << std::endl;
	if (!state.motd.empty())
		std::cout << state.motd << "\n" << std::endl;
	else
		std::cout << "empty" << "\n" << std::endl;

	// Starting time
	displayFullTime(state.start_time);
}
