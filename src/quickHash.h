#pragma once
#include "static.h"

//MurmurHashNeutral2 modified
inline void quickHash(const void * key, std::size_t len, char* out, uint32_t seed = 0, std::size_t mIndex = 0)
{
	const uint32_t ms[10] = 
	{	0X5BD1E995, 0X19D699A5, 0XB11924E1, 0X16118B03, 0X53C93455,
		0x42077030, 0xA8CFBD44, 0xA708AFE2, 0x45D8BF56, 0x818DDC0
	};
	const uint32_t m = ms[mIndex];
	const int r = 24;

	uint32_t h = seed ^ len;

	const unsigned char * data = (const unsigned char *)key;

	while (len >= 4)
	{
		uint32_t k;

		k = data[0];
		k |= data[1] << 8;
		k |= data[2] << 16;
		k |= data[3] << 24;

		k *= m;
		k ^= k >> r;
		k *= m;

		h *= m;
		h ^= k;

		data += 4;
		len -= 4;
	}

	switch (len)
	{
	case 3: h ^= data[2] << 16;
	case 2: h ^= data[1] << 8;
	case 1: h ^= data[0];
		h *= m;
	};

	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	*out++ = h >> 24;
	*out++ = h >> 16;
	*out++ = h >> 8;
	*out = h;
}

inline void quickHash320(const void * key, int len, char* out, uint32_t seed=0)
{
	for (unsigned int i = 0; i < 10; ++i)
	{
		quickHash(key, len, out+i*4, seed, i);
	}
}