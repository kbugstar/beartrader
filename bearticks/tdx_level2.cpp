#include "tdx_level2.h"



TdxLevel2::TdxLevel2()
{
	sequence_base_ = 0;
}


TdxLevel2::~TdxLevel2()
{
}

bool TdxLevel2::Login()
{
	unsigned char* buf = new unsigned char[280];

	int offset = buildTdxHeader(getSequence(6), 0x7b, 282, buf);


}

uint16_t TdxLevel2::getSequence(int head)
{
	if (sequence_base_ == 0)
		sequence_base_ = head << 10;
	uint16_t n = (sequence_base_ & 0x3ff) + 1;
	if (n > 0x3ff)
		n = 1;
	n += (head << 10);
	sequence_base_ = n;
	return n;
}

uint16_t TdxLevel2::buildTdxHeader(uint16_t sequence, uint16_t request_type, uint16_t data_len, unsigned char* buf)
{
	buf[0] = 0xc;
	*(uint16_t*)&buf[1] = sequence;
	*(uint16_t*)&buf[3] = request_type;
	buf[5] = 1;
	*(uint16_t*)&buf[6] = data_len;
	*(uint16_t*)&buf[8] = data_len;
	return 10;
}

