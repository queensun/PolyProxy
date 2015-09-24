#pragma once
#include "Session.h"
#include "RelaySvc.h"

#include <boost/logic/tribool.hpp>

class HttpsProxySeesion
	: public Session
{
public:
	HttpsProxySeesion(
		std::shared_ptr<UpStreamProxy> upStreamProxy,
		std::shared_ptr<ProxySvc::Acl> acl,
		std::unique_ptr<Stream>&& stream)
		: upStreamProxy_(upStreamProxy)
		, acl_(acl)
		, Session(std::move(stream))
	{}

	virtual void go();

protected:
	class InvalidProtocol
	{};

private:
	void writeResponse(unsigned int statusCode, const std::string& statusText, boost::asio::yield_context yield);
	static boost::asio::ip::address stringToAddress(const std::string& str);
	std::shared_ptr<UpStreamProxy> upStreamProxy_;
	std::shared_ptr<ProxySvc::Acl> acl_;
};