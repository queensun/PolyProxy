#pragma once
#include "Stream.h"
#include "HostStatistics.h"

class TrialStream
	: public Stream
{
public:
	TrialStream(std::unique_ptr<Stream> nextLayer,
		std::shared_ptr<HostStatistics> statistics)
		: nextLayer_(std::move(nextLayer))
		, statistics_(statistics)
	{}

	virtual void readSome(const MutableBuffers& buffers, const ReadHandler& handler);
	virtual void writeSome(const ConstBuffers& buffers, const WriteHandler& handler);
	virtual boost::system::error_code shutdown(
		boost::asio::ip::tcp::socket::shutdown_type op, boost::asio::yield_context yield)
	{
		return nextLayer_->shutdown(op, yield);
	}
	virtual void cancel()
	{
		nextLayer_->cancel();
	}
	virtual void close(boost::asio::yield_context yield)
	{
		nextLayer_->close(yield);
	}
	virtual PeerID localPeerID() const
	{
		return nextLayer_->localPeerID();
	}
	virtual PeerID remotePeerID() const
	{
		return nextLayer_->remotePeerID();
	}

protected:
	std::shared_ptr<HostStatistics> statistics_;
	std::unique_ptr<Stream> nextLayer_;
};