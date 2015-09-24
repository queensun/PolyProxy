#pragma once
#include "static.h"

using Host = boost::variant < boost::asio::ip::address, std::string > ;

//ip4 < ip6 < string
inline bool operator < (const Host& h1, const Host& h2)
{
	if (boost::get<std::string>(&h1))
	{
		if (boost::get<std::string>(&h2))
		{
			return boost::get<std::string>(h1) < boost::get<std::string>(h2);
		}
		else
		{
			return false;
		}
	}
	else	//h1=ip
	{
		if (boost::get<std::string>(&h2))
		{
			return true;
		}
		else
		{
			const boost::asio::ip::address& ip1 = boost::get<boost::asio::ip::address>(h1);
			const boost::asio::ip::address& ip2 = boost::get<boost::asio::ip::address>(h2);

			if (ip1.is_v6())
			{
				if (ip2.is_v6())
				{
					return std::memcmp(
						ip1.to_v6().to_bytes().data(),
						ip2.to_v6().to_bytes().data(),
						16) < 0;
				}
				else
				{
					return false;
				}
			}
			else //h1=ipv4
			{
				if (ip2.is_v6())
				{
					return true;
				}
				else
				{
					return ip1.to_v4().to_ulong() < ip2.to_v4().to_ulong();
				}
			}
		}
	}
}