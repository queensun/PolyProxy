#pragma once
#include "UpStreamProxy.h"
#include "Obfs.h"

class ObfsUpStreamProxy
	: public UpStreamProxy
{
public:
	ObfsUpStreamProxy(const std::string& key)
		: obfsKey_(key)
	{}
	virtual std::unique_ptr<Stream> connect(
		const std::string& host, unsigned short port, boost::asio::yield_context yield);
	virtual std::unique_ptr<Stream> connect(
		const boost::asio::ip::address& address, unsigned short port, boost::asio::yield_context yield);

protected:
	Obfs::Key obfsKey_;
};

class LANObfsUpStreamProxy
	: public LANUpStreamProxy
{
public:
	LANObfsUpStreamProxy(const std::string& key)
		: obfsKey_(key)
	{}
	virtual std::unique_ptr<Stream> connect(
		const std::string& host, unsigned short port, boost::asio::yield_context yield);
	virtual std::unique_ptr<Stream> connect(
		const boost::asio::ip::address& address, unsigned short port, boost::asio::yield_context yield);

protected:
	Obfs::Key obfsKey_;
};