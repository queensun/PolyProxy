#include "AllProxySvc.h"
#include "AllProxySession.h"

void AllProxySvc::killInnerEntry(InnerEntry* innerEntry)
{
	*std::find(innerEntries_.begin(), innerEntries_.end(), innerEntry)=0;
}

void AllProxySvc::link(InnerEntryEnum innerEntryEnum, std::shared_ptr<Svc> svc)
{
	InnerEntry*& innerEntry = innerEntries_[static_cast<std::size_t>(innerEntryEnum)];
	if (innerEntry)
	{
		throw InvalidAssignmentExcept();
	}

	innerEntry = new InnerEntry(
		std::static_pointer_cast<Lobby>(shared_from_this()));
	svc->insertEntry(std::unique_ptr<Entry>(innerEntry));
}

void AllProxySvc::createSession(std::unique_ptr<Stream> stream)
{
	std::shared_ptr<Session> session(
		new AllProxySession(std::move(stream), std::static_pointer_cast<AllProxySvc>(shared_from_this())));
	session->go();
}