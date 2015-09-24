#pragma once
#include "static.h"
#include "Stream.h"

class UpStreamProxy
{
public:
	virtual std::unique_ptr<Stream> connect(
		const std::string& host, unsigned short port, boost::asio::yield_context yield);
	virtual std::unique_ptr<Stream> connect(
		const boost::asio::ip::address& address, unsigned short port, boost::asio::yield_context yield);
	virtual boost::asio::ip::address resolve(
		const std::string& host, boost::asio::yield_context yield);
	virtual std::string resolve(
		const boost::asio::ip::address& address, boost::asio::yield_context yield);

	unsigned int timeout() const
	{
		return timeout_;
	}

	void timeout(unsigned int milliseconds)
	{
		timeout_ = milliseconds;
	}

protected:
	unsigned int timeout_ = 0;
};

class LANUpStreamProxy
	: public UpStreamProxy
{
public:
	virtual std::unique_ptr<Stream> connect(
		const std::string& host, unsigned short port, boost::asio::yield_context yield);
	virtual std::unique_ptr<Stream> connect(
		const boost::asio::ip::address& address, unsigned short port, boost::asio::yield_context yield);
};

class Connector
{
public:
	virtual std::unique_ptr<Stream> operator()(boost::asio::yield_context yield) = 0;
};

template <typename A>
class ConnectorT
	: public Connector
{
public:
	ConnectorT(const A& a, unsigned short port, std::shared_ptr<UpStreamProxy> upStreamProxy)
		: a_(a), port_(port), upStreamProxy_(upStreamProxy)
	{}

	virtual std::unique_ptr<Stream> operator()(boost::asio::yield_context yield)
	{
		return upStreamProxy_->connect(a_, port_, yield);
	}

private:
	A a_;
	unsigned short port_;
	std::shared_ptr<UpStreamProxy> upStreamProxy_;
};

extern std::shared_ptr<UpStreamProxy> defaultUpStreamProxy;

template <typename A>
inline std::unique_ptr<Connector> createConnector(const A& a, unsigned short port,
	std::shared_ptr<UpStreamProxy> upStreamProxy = defaultUpStreamProxy)
{
	return std::unique_ptr<Connector>(new ConnectorT<A>(a, port, upStreamProxy));
}

inline std::unique_ptr<Connector> createConnector(const char* host, unsigned short port,
	std::shared_ptr<UpStreamProxy> upStreamProxy = defaultUpStreamProxy)
{
	return std::unique_ptr<Connector>(new ConnectorT<std::string>(host, port, upStreamProxy));
}

