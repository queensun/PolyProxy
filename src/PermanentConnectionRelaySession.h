#pragma once
#include "Session.h"

class PermanentConnection;
class PermanentConnectionRelaySession
	: public Session
{
public:
	PermanentConnectionRelaySession(
		std::shared_ptr<PermanentConnection> connection,
		const std::string& virtualService,
		std::unique_ptr<Stream>&& stream)
		: connection_(connection)
		, virtualService_(virtualService)
		, Session(std::move(stream))
	{}

	virtual void go();

protected:
	std::string virtualService_;
	std::shared_ptr<PermanentConnection> connection_;
};

