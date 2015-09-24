#include "PermanentConnectionRelaySvc.h"
#include "PermanentConnectionRelaySession.h"
#include "PermanentConnection.h"

void PermanentConnectionRelaySvc::createSession(std::unique_ptr<Stream> stream)
{
	if (connection_)
	{
		if (connection_->valid())
		{
			std::shared_ptr<Session> session(
				new PermanentConnectionRelaySession(connection_, virtualService_, std::move(stream)));
			session->go();
		}
		else
		{
			connection_.reset();
		}
	}
}
