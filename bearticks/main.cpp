#include <iostream>
#include <list>
#include "tdx_ticks.h"
#include "zlib.h"

int main(int argc, char** argv)
{
	TdxTicks ticks;
	ticks.parse_ticks(peer1_22, sizeof(peer1_22));
	return 0;
	
	for (int i = 0; i < kTdxServerCount; i++)
	{
		TdxTicks ticks;
		std::cout << kTdxServers[i].server_name << std::endl;
		if (ticks.connect_server(kTdxServers[i].ip_str, kTdxServers[i].ip_port))
		{
			std::cout << "Connect " << kTdxServers[i].ip_str << " successed!" << std::endl;
			ticks.get_ticks();
		}
		else
		{
			std::cout << "Connect " << kTdxServers[i].ip_str << " failed!" << std::endl;
		}
	}
	return 0;
}