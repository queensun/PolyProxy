#pragma once
#include "Session.h"
#include "RelaySvc.h"

#include <boost/logic/tribool.hpp>

class Socks5Session
	: public Session
{
public:
	Socks5Session(
		std::shared_ptr<UpStreamProxy> upStreamProxy,
		std::shared_ptr<ProxySvc::Acl> acl,
		std::unique_ptr<Stream>&& stream)
		: upStreamProxy_(upStreamProxy)
		, acl_(acl)
		, Session(std::move(stream))
	{}

	virtual void go();

protected:
	void readMethod(boost::asio::yield_context yield);
	void readRequest(boost::asio::yield_context yield);
	void handleConnectRequest(boost::asio::yield_context yield);
	unsigned short readPort(boost::asio::yield_context yield);
	void readVersion(boost::asio::yield_context yield);
	void writeCommandNotSupoorted(boost::asio::yield_context yield);
	void writeError(unsigned char errorCode, boost::asio::yield_context yield);

	class InvalidProtocol
	{};

private:
	std::shared_ptr<UpStreamProxy> upStreamProxy_;
	std::shared_ptr<ProxySvc::Acl> acl_;
};