#pragma once
#include "static.h"
#include "Svc.h"
#include "Entry.h"
#include "InterCoroutineVar.h"

#include <memory>

class Lobby;

class InnerEntry
	: public Entry
{
public:
	InnerEntry(std::shared_ptr<Lobby> lobby)
		:lobby_(lobby)
	{}
	void passStream(std::unique_ptr<Stream> stream, boost::asio::yield_context yield);
	virtual std::unique_ptr<Stream> accept(boost::asio::yield_context yield);
	~InnerEntry();

protected:
	virtual void doStart();
	virtual void doStop(boost::asio::yield_context yield);

private:
	using Streams = InterCoroutineVar < std::unique_ptr<Stream> > ;
	Streams streams_;
	std::shared_ptr<Lobby> lobby_;
};

class Lobby
	:public Svc
{
public:
	virtual void killInnerEntry(InnerEntry* innerEntry)
	{}
};