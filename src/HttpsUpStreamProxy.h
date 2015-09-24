#pragma once
#include "UpStreamProxy.h"

class HttpsUpStreamProxy
	: public UpStreamProxy
{
public:
	HttpsUpStreamProxy(std::unique_ptr<Connector> &&connector)
		: connector_(std::move(connector))
	{}

	virtual std::unique_ptr<Stream> connect(
		const std::string& host, unsigned short port, boost::asio::yield_context yield);
	virtual std::unique_ptr<Stream> connect(
		const boost::asio::ip::address& address, unsigned short port, boost::asio::yield_context yield);

protected:
	std::unique_ptr<Connector> connector_;
};