#pragma once

#include "IpPool.h"
#include "SslContext.h"
#include "SslUpStreamProxy.h"

class SiteScanner
	: public std::enable_shared_from_this<SiteScanner>
{
public:
	SiteScanner(std::shared_ptr<UpStreamProxy> upStreamProxy)
		: upStreamProxy_(upStreamProxy)
	{}
	virtual ~SiteScanner()
	{}

	inline void addRanges(const std::string& str)
	{
		ipPool_.addRanges(str);
	}

	inline void addRange(const std::string& str)
	{
		ipPool_.addRange(str);
	}

	void go();

	inline std::size_t running() const
	{
		return running_;
	}

	inline std::size_t concurrence() const
	{
		return concurrence_;
	}

	inline void concurrence(std::size_t concurrence)
	{
		concurrence_ = concurrence;
	}

	inline unsigned short port()
	{
		return port_;
	}

	void port(unsigned short v)
	{
		port_ = v;
	}

protected:
	IpPool ipPool_;
	unsigned short port_ = 443;
	std::size_t running_ = 0;
	std::size_t concurrence_ = 16;
	std::shared_ptr<UpStreamProxy> upStreamProxy_;

	virtual void check(const boost::asio::ip::address_v4& ip, boost::asio::yield_context yield) = 0;
};