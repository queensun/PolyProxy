#include "PermanentConnectionEntry.h"
#include "PermanentConnection.h"

void PermanentConnectionEntry::passStream(std::unique_ptr<Stream> stream, boost::asio::yield_context yield)
{
	incomingStreams_.push(std::move(stream));
}
std::unique_ptr<Stream> PermanentConnectionEntry::accept(
	boost::asio::yield_context yield)
{
	std::unique_ptr<Stream> ret;
	do
	{
		try
		{
			ret = incomingStreams_.pop(yield);
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

void PermanentConnectionEntry::doStop(boost::asio::yield_context yield)
{
	incomingStreams_.end();
	connection_->removeVirtualService(name_);
}
