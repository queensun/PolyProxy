#pragma once
#include "static.h"

class IpPool
{
public:
	IpPool()
	{}

	void addRange(const std::string str);
	void addRanges(const std::string str);
	boost::asio::ip::address_v4 random() const;

private:
	std::vector < std::pair<boost::asio::ip::address_v4, boost::asio::ip::address_v4> > data_;
};