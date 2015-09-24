#pragma once
#include "static.h"
#include "Lobby.h"
#include "Obfs.h"

class ObfsSvc
	: public Lobby
{
public:
	ObfsSvc(const std::string& key)
		: key_(key)
	{}

	void link(std::shared_ptr<Svc> svc);
	virtual void killInnerEntry(InnerEntry* innerEntry);
	inline const Obfs::Key& key() const
	{
		return key_;
	}
	inline void passStream(std::unique_ptr<Stream> stream, boost::asio::yield_context yield)
	{
		innerEntry_->passStream(std::move(stream), yield);
	}

protected:
	virtual void createSession(std::unique_ptr<Stream> stream);
	Obfs::Key key_;
	InnerEntry *innerEntry_;
};