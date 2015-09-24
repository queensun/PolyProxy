#pragma once
#include "Session.h"
#include "RelaySvc.h"
#include "UpStreamProxy.h"

class RelaySession
	: public Session
{
public:
	RelaySession(std::shared_ptr<Connector> connector,
		std::unique_ptr<Stream>&& stream)
		: connector_(connector)
		, Session(std::move(stream))
	{}

	virtual void go();	
private:
	std::shared_ptr<Connector> connector_;
};

void relay(std::shared_ptr<Stream> stream1, std::shared_ptr<Stream> stream2);