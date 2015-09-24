#include "Lobby.h"

void InnerEntry::passStream(std::unique_ptr<Stream> stream, boost::asio::yield_context yield)
{
	streams_.push(std::move(stream));
}
std::unique_ptr<Stream> InnerEntry::accept(boost::asio::yield_context yield)
{
	std::unique_ptr<Stream> ret;
	do
	{
		try
		{
			ret = streams_.pop(yield);
		}
		catch (InterCoroutineVarEndedExcept&)
		{
			return nullptr;
		}

		if (status() == Status::stopped || status() == Status::stopping)
		{
			return nullptr;
		}
	} while (status() == Status::paused);
		
	return std::move(ret);
}

void InnerEntry::doStart()
{
}

void InnerEntry::doStop(boost::asio::yield_context yield)
{
	streams_.end();
}

InnerEntry::~InnerEntry()
{
	lobby_->killInnerEntry(this);
}