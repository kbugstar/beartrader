#include "tdx_decoder.h"

int TdxDecoder::ReadInt(const unsigned char* Buffer, size_t Size, size_t* NumUsed)
{
	int ret = 0;
	size_t i = 0;
	int shift_bit = 6;
	bool is_nagtive = Buffer[i] & 0x40;
	ret = Buffer[i] & 0x3f;
	while ((Buffer[i++] & 0x80) && i < Size)
	{
		ret += (Buffer[i] & 0x7f) << shift_bit;
		shift_bit += 7;
	}
	if (is_nagtive)
	{
		ret = -ret;
	}
	*NumUsed = i;
	return ret;
}


