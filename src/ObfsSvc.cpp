#include "ObfsSvc.h"
#include "ObfsSession.h"

void ObfsSvc::createSession(std::unique_ptr<Stream> stream)
{
	std::shared_ptr<Session> session(
		new ObfsSession(std::move(stream), std::static_pointer_cast<ObfsSvc>(shared_from_this())));
	session->go();
}

void ObfsSvc::link(std::shared_ptr<Svc> svc)
{
	innerEntry_ = new InnerEntry(std::static_pointer_cast<Lobby>(shared_from_this()));
	svc->insertEntry(std::unique_ptr<Entry>(innerEntry_));
}

void ObfsSvc::killInnerEntry(InnerEntry* innerEntry)
{
	assert(innerEntry_ = innerEntry);
	stop();
}