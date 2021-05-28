#include "BufferCopy/WriteToRDRAM.h"
#include <arm_neon.h>

template <>
void writeToRdram<u32,u16>(u32* _src, u16* _dst,
	u16(*converter)(u32 _c, u32 x, u32 y),
	bool(*tester)(u32 _c),
	u32 _xor,
	u32 _width,
	u32 _height,
	u32 _numPixels,
	u32 _startAddress,
	u32 _bufferAddress,
	u32 _bufferSize)
{
	(void)converter;
	(void)tester;
	(void)_height;

	u32 chunkStart = ((_startAddress - _bufferAddress) >> (_bufferSize - 1)) % _width;
	if (chunkStart % 2 != 0) {
		--chunkStart;
		--_dst;
		++_numPixels;
	}

	u32 numStored = 0;
	u32 y = 0;
	uint8x8x4_t vsrc;
	uint16x8_t vdst;
	uint8x8_t vzero = vcreate_u8(0);

	if (chunkStart > 0)
		_src += chunkStart;

	u32 _leftover = _numPixels % 8;
	while (_numPixels >= 8 && numStored < (_numPixels - _leftover))
	{
		vsrc = vld4_u8((u8*)_src);
		vsrc.val[3] = vcgt_u8(vsrc.val[3], vzero);
		vdst = vshll_n_u8(vsrc.val[0], 8);							// R
		vdst = vsriq_n_u16(vdst, vshll_n_u8(vsrc.val[1], 8), 5);	// G
		vdst = vsriq_n_u16(vdst, vshll_n_u8(vsrc.val[2], 8), 10);	// B
		vdst = vsriq_n_u16(vdst, vshll_n_u8(vsrc.val[3], 8), 15);	// A
		vdst = vrev32q_u16(vdst);
		vst1q_u16(_dst, vdst);
		_src+=8;
		_dst+=8;
		numStored+=8;
	}

	while(_leftover)
	{
		vsrc = vld4_lane_u8((u8*)_src, vsrc, 0);
		vsrc.val[3] = vcgt_u8(vsrc.val[3], vzero);
		vdst = vshll_n_u8(vsrc.val[0], 8);							// R
		vdst = vsriq_n_u16(vdst, vshll_n_u8(vsrc.val[1], 8), 5);	// G
		vdst = vsriq_n_u16(vdst, vshll_n_u8(vsrc.val[2], 8), 10);	// B
		vdst = vsriq_n_u16(vdst, vshll_n_u8(vsrc.val[3], 8), 15);	// A
		vst1q_lane_u16(&_dst[numStored ^ _xor], vdst, 0);
		_src++;
		numStored++;
		_leftover--;
	}
}