#include <stdint.h>
#include "CTdxTicks.h"
#include "zlib.h"

#pragma comment(lib, "ws2_32.lib")
//#pragma comment(lib, "zlibwapi.lib")

#define START_UP_COMMAND_1 "\x0c\x02\x18\x93\x00\x01\x03\x00\x03\x00\x0d\x00\x01"
#define START_UP_COMMAND_2 "\x0c\x02\x18\x94\x00\x01\x03\x00\x03\x00\x0d\x00\x02"
#define START_UP_COMMAND_3 "\x0c\x03\x18\x99\x00\x01\x20\x00\x20\x00\xdb\x0f\xd5\xd0\xc9\xcc\xd6\xa4\xc8\xaf\x00\x00\x00\x8f\xc2\x25\x40\x13\x00\x00\xd5\x00\xc9\xcc\xbd\xf0\xd7\xea\x00\x00\x00\x02"

data_block _hand_shake_commands[] = {
	{ START_UP_COMMAND_1, sizeof(START_UP_COMMAND_1) - 1 },
	{ START_UP_COMMAND_2, sizeof(START_UP_COMMAND_2) - 1 },
	{ START_UP_COMMAND_3, sizeof(START_UP_COMMAND_3) - 1 }
};


server_info _tdx_servers[] =
{
	{ "长城国瑞电信1", "218.85.139.19", 7709 },
	{ "长城国瑞电信2", "218.85.139.20", 7709 },
	{ "长城国瑞网通", "58.23.131.163", 7709 },
	{ "上证云成都电信一", "218.6.170.47", 7709 },
	{ "上证云北京联通一", "123.125.108.14", 7709 },
	{ "上海电信主站Z1", "180.153.18.170", 7709 },
	{ "上海电信主站Z2", "180.153.18.171", 7709 },
	{ "上海电信主站Z80", "180.153.18.172", 80 },
	{ "北京联通主站Z1", "202.108.253.130", 7709 },
	{ "北京联通主站Z2", "202.108.253.131", 7709 },
	{ "北京联通主站Z80", "202.108.253.139", 80 },
	{ "杭州电信主站J1", "60.191.117.167", 7709 },
	{ "杭州电信主站J2", "115.238.56.198", 7709 },
	{ "杭州电信主站J3", "218.75.126.9", 7709 },
	{ "杭州电信主站J4", "115.238.90.165", 7709 },
	{ "杭州联通主站J1", "124.160.88.183", 7709 },
	{ "杭州联通主站J2", "60.12.136.250", 7709 },
	{ "杭州华数主站J1", "218.108.98.244", 7709 },
	{ "杭州华数主站J2", "218.108.47.69", 7709 },
	{ "义乌移动主站J1", "223.94.89.115", 7709 },
	{ "青岛联通主站W1", "218.57.11.101", 7709 },
	{ "青岛电信主站W1", "58.58.33.123", 7709 },
	{ "深圳电信主站Z1", "14.17.75.71", 7709 },
	{ "云行情上海电信Z1", "114.80.63.12", 7709 },
	{ "云行情上海电信Z2", "114.80.63.35", 7709 },
	{ "上海电信主站Z3", "180.153.39.51", 7709 },
	{ "招商证券深圳行情", "119.147.212.81", 7709 },
	{ "华泰证券{南京电信}", "221.231.141.60", 7709 },
	{ "华泰证券{上海电信}", "101.227.73.20", 7709 },
	{ "华泰证券{上海电信二}", "101.227.77.254", 7709 },
	{ "华泰证券{深圳电信}", "14.215.128.18", 7709 },
	{ "华泰证券{武汉电信}", "59.173.18.140", 7709 },
	{ "华泰证券{天津联通}", "60.28.23.80", 7709 },
	{ "华泰证券{沈阳联通}", "218.60.29.136", 7709 },
	{ "华泰证券{南京联通}", "122.192.35.44", 7709 },
	{ "华泰证券{南京联通}", "122.192.35.44", 7709 },
	{ "安信", "112.95.140.74", 7709 },
	{ "安信", "112.95.140.92", 7709 },
	{ "安信", "112.95.140.93", 7709 },
	{ "安信", "114.80.149.19", 7709 },
	{ "安信", "114.80.149.21", 7709 },
	{ "安信", "114.80.149.22", 7709 },
	{ "安信", "114.80.149.91", 7709 },
	{ "安信", "114.80.149.92", 7709 },
	{ "安信", "121.14.104.60", 7709 },
	{ "安信", "121.14.104.66", 7709 },
	{ "安信", "123.126.133.13", 7709 },
	{ "安信", "123.126.133.14", 7709 },
	{ "安信", "123.126.133.21", 7709 },
	{ "安信", "211.139.150.61", 7709 },
	{ "安信", "59.36.5.11", 7709 },
	{ "广发", "119.29.19.242", 7709 },
	{ "广发", "123.138.29.107", 7709 },
	{ "广发", "123.138.29.108", 7709 },
	{ "广发", "124.232.142.29", 7709 },
	{ "广发", "183.57.72.11", 7709 },
	{ "广发", "183.57.72.12", 7709 },
	{ "广发", "183.57.72.13", 7709 },
	{ "广发", "183.57.72.15", 7709 },
	{ "广发", "183.57.72.21", 7709 },
	{ "广发", "183.57.72.22", 7709 },
	{ "广发", "183.57.72.23", 7709 },
	{ "广发", "183.57.72.24", 7709 },
	{ "广发", "183.60.224.177", 7709 },
	{ "广发", "183.60.224.178", 7709 },
	{ "国泰君安", "113.105.92.100", 7709 },
	{ "国泰君安", "113.105.92.101", 7709 },
	{ "国泰君安", "113.105.92.102", 7709 },
	{ "国泰君安", "113.105.92.103", 7709 },
	{ "国泰君安", "113.105.92.104", 7709 },
	{ "国泰君安", "113.105.92.99", 7709 },
	{ "国泰君安", "117.34.114.13", 7709 },
	{ "国泰君安", "117.34.114.14", 7709 },
	{ "国泰君安", "117.34.114.15", 7709 },
	{ "国泰君安", "117.34.114.16", 7709 },
	{ "国泰君安", "117.34.114.17", 7709 },
	{ "国泰君安", "117.34.114.18", 7709 },
	{ "国泰君安", "117.34.114.20", 7709 },
	{ "国泰君安", "117.34.114.27", 7709 },
	{ "国泰君安", "117.34.114.30", 7709 },
	{ "国泰君安", "117.34.114.31", 7709 },
	{ "国信", "182.131.3.252", 7709 },
	{ "国信", "183.60.224.11", 7709 },
	{ "国信", "58.210.106.91", 7709 },
	{ "国信", "58.63.254.216", 7709 },
	{ "国信", "58.63.254.219", 7709 },
	{ "国信", "58.63.254.247", 7709 },
	{ "海通", "123.125.108.90", 7709 },
	{ "海通", "175.6.5.153", 7709 },
	{ "海通", "182.118.47.151", 7709 },
	{ "海通", "182.131.3.245", 7709 },
	{ "海通", "202.100.166.27", 7709 },
	{ "海通", "222.161.249.156", 7709 },
	{ "海通", "42.123.69.62", 7709 },
	{ "海通", "58.63.254.191", 7709 },
	{ "海通", "58.63.254.217", 7709 },
	{ "华林", "120.55.172.97", 7709 },
	{ "华林", "139.217.20.27", 7709 },
	{ "华林", "202.100.166.21", 7709 },
	{ "华林", "202.96.138.90", 7709 },
	{ "华林", "218.106.92.182", 7709 },
	{ "华林", "218.106.92.183", 7709 },
	{ "华林", "220.178.55.71", 7709 },
	{ "华林", "220.178.55.86", 7709 }
};

const uint32_t _tdx_servers_count ARRAYSIZE(_tdx_servers);

CTdxTicks::CTdxTicks()
{
}

CTdxTicks::~CTdxTicks()
{
}

bool CTdxTicks::connect_server(const char* ip, uint16_t port)
{
	WORD ver;
	WSADATA data;
	ver = MAKEWORD(2, 0);
	WSAStartup(ver, &data);

	s_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s_ == INVALID_SOCKET)
	{
		return false;
	}

	struct timeval tv;
	tv.tv_sec = 3;

	setsockopt(s_, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
	setsockopt(s_, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(tv));

	sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.S_un.S_addr = inet_addr(ip);

	if (SOCKET_ERROR == ::connect(s_, (sockaddr*)&addr, sizeof(addr)))
	{
		closesocket(s_);
		return false;
	}

	for (auto x : _hand_shake_commands)
	{
		if (SOCKET_ERROR == send(s_, x.data, x.size, 0))
			return false;
		tdx_header header;
		if (SOCKET_ERROR == recv(s_, (char*)&header, sizeof(header), 0))
			return false;
		if (header.ziped_size == 0)
			return false;
		char *buf = new char[header.ziped_size];
		size_t recv_bytes = recv(s_, buf, header.ziped_size, 0);
		if (SOCKET_ERROR == recv_bytes)
		{
			delete[] buf;
			return false;
		}
		delete[] buf;
	}

	return true;
}

bool CTdxTicks::get_ticks()
{
	const char query_cmd[] = "\x0c\x17\x08\x01\x01\x01\x0e\x00\x0e\x00\xc5\x0f\x00\x00";
	char buf[1024];

	size_t size = sizeof(query_cmd) - 1;
	memcpy(buf, query_cmd, size);
	memcpy(buf + size, "000001", 6);
	size += 6;
	*(uint16_t*)&buf[size] = 0;
	size += 2;
	*(uint16_t*)&buf[size] = 100;
	size += 2;
	int ret = send(s_, buf, size, 0);
	if (ret == SOCKET_ERROR)
	{
		return false;
	}
	tdx_header header;
	if (SOCKET_ERROR == recv(s_, (char*)&header, sizeof(header), 0))
		return false;
	if (header.ziped_size == 0)
		return false;
	char *recv_buf = new char[header.ziped_size];
	size_t recv_bytes = recv(s_, recv_buf, header.ziped_size, 0);
	if (SOCKET_ERROR == recv_bytes)
	{
		delete[] recv_buf;
		return false;
	}
	if (header.ziped_size != header.unzip_size)
	{
		char *unzip_buf = new char[header.unzip_size + 16];
		size_t out_size = header.unzip_size + 16;
		uncompress((Bytef*)unzip_buf, (uLongf*)&out_size, (Bytef*)recv_buf, recv_bytes);
		delete[] unzip_buf;
	}
	delete[] recv_buf;
	return true;
}
