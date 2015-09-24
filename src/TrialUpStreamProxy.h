#pragma once
#include "static.h"
#include "UpStreamProxy.h"
#include "HostPool.h"

class TrialUpStreamProxy
	: public UpStreamProxy
{
public:
	TrialUpStreamProxy(
		std::shared_ptr<UpStreamProxy> nextLayer, 
		std::shared_ptr<HostPool> pool)
		: nextLayer_(nextLayer)
		, pool_(pool)
	{}

	virtual std::unique_ptr<Stream> connect(
		const std::string& host, unsigned short port, boost::asio::yield_context yield);
	virtual std::unique_ptr<Stream> connect(
		const boost::asio::ip::address& address, unsigned short port, boost::asio::yield_context yield);

protected:
	std::shared_ptr<HostPool> pool_;
	std::shared_ptr<UpStreamProxy> nextLayer_;
};