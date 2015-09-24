#pragma once

#include "SiteScanner.h"
#include "GoogleSslUpStreamProxy.h"
#include "GoogleHostPool.h"
#include "TrialUpStreamProxy.h"

class GoogleScanner
	: public SiteScanner
{
public:
	GoogleScanner(
		std::shared_ptr<GoogleSslUpStreamProxy> googleSslUpStreamProxy,
		std::shared_ptr<GoogleHostPool> pool)
		: SiteScanner(std::make_shared<TrialUpStreamProxy>(googleSslUpStreamProxy, pool))
		, hostPool_(pool)
	{}

protected:
	std::shared_ptr<GoogleHostPool> hostPool_;
	virtual void check(const boost::asio::ip::address_v4& ip, boost::asio::yield_context yield);

	bool valid(
		const boost::asio::ip::address_v4& ip,
		const std::string& hostname,
		boost::asio::yield_context yield);

};