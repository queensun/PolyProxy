#pragma once

#include "static.h"

template <typename T>
inline std::array<char, sizeof(T)> toBytes(T t)
{
	std::array<char, sizeof(T)> ret;
	for (std::size_t i = 0; i < ret.size(); ++i)
	{
		ret[i] = t >> ((sizeof(T) - i - 1) * 8);
	}

	return ret;
}

template <std::size_t SIZE, typename T>
inline std::array<char, SIZE> toBytes(T t)
{
	std::array<char, SIZE> ret;
	for (std::size_t i = 0; i < ret.size(); ++i)
	{
		ret[i] = t >> ((SIZE - i - 1) * 8);
	}

	return ret;
}