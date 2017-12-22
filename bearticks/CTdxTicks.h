#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>

#pragma pack(push)
#pragma pack(1)
typedef struct _tdx_header
{
	uint32_t a;
	uint32_t b;
	uint32_t c;
	uint16_t ziped_size;
	uint16_t unzip_size;
}tdx_header;
#pragma pack(pop)

typedef struct _data_block
{
	const char* data;
	size_t		size;
}data_block;

typedef struct _server_info
{
	const char* server_name;
	const char* ip_str;
	uint16_t	ip_port;
}server_info;


extern const uint32_t _tdx_servers_count;
extern server_info _tdx_servers[];

class CTdxTicks
{
public:
	CTdxTicks();
	~CTdxTicks();

	bool connect_server(const char* ip, uint16_t port);

	bool get_ticks();

private:
	SOCKET s_;
};

