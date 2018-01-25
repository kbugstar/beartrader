#pragma once
#include <stdint.h>

class TdxLevel2
{
public:
	TdxLevel2();
	~TdxLevel2();

public:
	bool Login();

private:
	uint16_t getSequence(int head);
	uint16_t buildTdxHeader(uint16_t sequence, uint16_t request_type, uint16_t data_len, unsigned char* buf);

private:

	uint16_t sequence_base_;
};

