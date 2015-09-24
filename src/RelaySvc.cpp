#include "RelaySvc.h"
#include "RelaySession.h"

void RelaySvc::createSession(std::unique_ptr<Stream> stream)
{
	std::shared_ptr<Session> session(
		new RelaySession(connector_, std::move(stream)));
	session->go();
}