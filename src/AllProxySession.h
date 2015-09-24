#pragma once

#include "static.h"
#include "AllProxySvc.h"
#include "Session.h"

class AllProxySession
	: public Session
{
public:
	AllProxySession(std::unique_ptr<Stream>&& stream, std::shared_ptr<AllProxySvc> svc)
		: Session(std::move(stream))
		, svc_(svc)
	{}

	void go();

protected:
	std::shared_ptr<AllProxySvc> svc_;
};