#include <iostream>
#include <list>
#include "CTdxTicks.h"
#include "zlib.h"

int main(int argc, char** argv)
{
	for (int i = 0; i < _tdx_servers_count; i++)
	{
		CTdxTicks ticks;
		std::cout << _tdx_servers[i].server_name << std::endl;
		if (ticks.connect_server(_tdx_servers[i].ip_str, _tdx_servers[i].ip_port))
		{
			std::cout << "Connect " << _tdx_servers[i].ip_str << " successed!" << std::endl;
			ticks.get_ticks();
		}
		else
		{
			std::cout << "Connect " << _tdx_servers[i].ip_str << " failed!" << std::endl;
		}
	}
	return 0;
}