#pragma once

#include "SslUpStreamProxy.h"

class GoogleSslUpStreamProxy
	: public SslUpStreamProxy
{
public:
	GoogleSslUpStreamProxy(std::shared_ptr<SslContext> context,
		std::shared_ptr<UpStreamProxy> parent = defaultUpStreamProxy)
		: SslUpStreamProxy(context, parent)
	{}

	virtual bool check(const PeerID& peerID);
};