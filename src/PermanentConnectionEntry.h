#pragma once
#include "Entry.h"
#include "Stream.h"
#include "InterCoroutineVar.h"

class PermanentConnection;
class PermanentConnectionEntry
	: public Entry
{
public:
	PermanentConnectionEntry(
		const std::string& name,
		std::shared_ptr<PermanentConnection> connection)
		: name_(name)
		, connection_(connection)
	{}
	void passStream(std::unique_ptr<Stream> stream, boost::asio::yield_context yield);
	virtual std::unique_ptr<Stream> accept(boost::asio::yield_context yield);

protected:
	virtual void doStart()
	{}
	virtual void doStop(boost::asio::yield_context yield);

	InterCoroutineVar<std::unique_ptr<Stream> > incomingStreams_;
	std::shared_ptr<PermanentConnection> connection_;
	std::string name_;
};