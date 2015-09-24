#include "IpPool.h"
#include <random>

void IpPool::addRanges(const std::string str)
{	
	std::size_t begin = 0;
	for (std::size_t position = position = str.find(' ', 0); 
		position != str.npos; position = str.find(' ', begin))
	{
		std::string range = str.substr(begin, position - begin);
		addRange(range);
		begin = position + 1;
	}
	std::string range = str.substr(begin, str.npos);
	addRange(range);
}

void IpPool::addRange(const std::string str)
{
	std::size_t slash = str.find('/');
	if (slash == str.npos)
	{
		boost::system::error_code ec;
		boost::asio::ip::address_v4 ip =
			boost::asio::ip::address_v4::from_string(str.substr(0, slash), ec);
		if (ec)
		{
			return;
		}
		boost::asio::ip::address_v4 ip2 = boost::asio::ip::address_v4(ip.to_ulong() + 1);
		std::pair<boost::asio::ip::address_v4, boost::asio::ip::address_v4> pair(ip, ip2);
		data_.push_back(pair);
	}
	else
	{
		boost::system::error_code ec;
		boost::asio::ip::address_v4 ip =
			boost::asio::ip::address_v4::from_string(str.substr(0, slash), ec);
		if (ec)
		{
			return;
		}
		unsigned int mask = std::atoi(str.substr(slash + 1, str.npos).c_str());
		if (mask > 31)
		{
			return;
		}
		boost::asio::ip::address_v4 ip2 =
			boost::asio::ip::address_v4((ip.to_ulong() | ((1 << (32-mask)) - 1)) + 1);
		std::pair<boost::asio::ip::address_v4, boost::asio::ip::address_v4> pair(ip, ip2);
		data_.push_back(pair);
	}
}

boost::asio::ip::address_v4 IpPool::random() const
{
	std::size_t size = 0;
	std::for_each(data_.begin(), data_.end(),
		[&size](std::pair<boost::asio::ip::address_v4, boost::asio::ip::address_v4> range)
	{
		size += range.second.to_ulong() - range.first.to_ulong();
	});

	static std::default_random_engine gen(::time(NULL));
	std::uniform_int_distribution<std::size_t> dist(0, size-1);
	std::size_t selected = dist(gen);
	boost::asio::ip::address_v4 ip;

	std::find_if(data_.begin(), data_.end(),
		[&selected, &ip](std::pair<boost::asio::ip::address_v4, boost::asio::ip::address_v4> range) ->bool
	{
		std::size_t count = range.second.to_ulong() - range.first.to_ulong();
		if (count < selected)
		{
			selected -= count;
			return false;
		}
		else
		{
			ip = boost::asio::ip::address_v4(range.first.to_ulong()+selected);
			return true;
		}
	});

	return ip;
}
