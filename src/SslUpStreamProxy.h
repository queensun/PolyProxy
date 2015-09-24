#pragma once
#include "UpStreamProxy.h"
#include "SslContext.h"

class SslUpStreamProxy
	: public UpStreamProxy
{
public:
	SslUpStreamProxy(std::shared_ptr<SslContext> context,
		std::shared_ptr<UpStreamProxy> parent=defaultUpStreamProxy)
		: context_(context), parent_(parent)
	{}
	virtual std::unique_ptr<Stream> connect(
		const std::string& host, unsigned short port, boost::asio::yield_context yield);
	virtual std::unique_ptr<Stream> connect(
		const boost::asio::ip::address& address, unsigned short port, boost::asio::yield_context yield);
	virtual boost::asio::ip::address resolve(
		const std::string& host, boost::asio::yield_context yield);
	virtual std::string resolve(
		const boost::asio::ip::address& address, boost::asio::yield_context yield);

	inline void sni(const std::string& v)
	{
		sni_ = v;
	}
	inline std::string sni() const
	{
		return sni_;
	}


protected:
	std::unique_ptr<Stream> connect(
		std::unique_ptr<Stream> stream, unsigned int timeout, boost::asio::yield_context yield);
	std::shared_ptr<SslContext> context_;
	std::shared_ptr<UpStreamProxy> parent_;
	std::string sni_;

	virtual bool check(const std::string& hostname, const PeerID& peerID);
	virtual bool check(const boost::asio::ip::address& address, const PeerID& peerID);
};

class HostSslUpStreamProxy
	: public SslUpStreamProxy
{
public:
	HostSslUpStreamProxy(std::shared_ptr<SslContext> context,
		const std::string& hostname,
		std::shared_ptr<UpStreamProxy> parent = defaultUpStreamProxy)
		: SslUpStreamProxy(context, parent)
		, hostname_(hostname)
	{}

protected:
	std::string hostname_;
	virtual bool check(const std::string& hostname, const PeerID& peerID);
	virtual bool check(const boost::asio::ip::address& address, const PeerID& peerID);
};
