#pragma once
#include "static.h"
#include "Stream.h"
#include "PermanentConnection.h"

class PermanentConnectionStream
	: public Stream
{
public:
	PermanentConnectionStream(
		std::uint32_t id, 
		PermanentConnection::StreamInfo& streamInfo,
		std::shared_ptr<PermanentConnection> connection,
		const PeerID& localPeerID,
		const PeerID&  remotePeerID
		)
		: id_(id)
		, streamInfo_(streamInfo)
		, connection_(connection)
		, localPeerID_(localPeerID)
		, remotePeerID_(remotePeerID)
	{}
	virtual ~PermanentConnectionStream();
	virtual void readSome(const MutableBuffers& buffers, const ReadHandler& handler);
	virtual void writeSome(const ConstBuffers& buffers, const WriteHandler& handler);
	virtual boost::system::error_code eof(boost::asio::yield_context yield);
	//virtual void cancel();
	virtual boost::system::error_code close(boost::asio::yield_context yield);
	virtual PeerID localPeerID() const;
	virtual PeerID remotePeerID() const;

private:
	bool closed_ = false;
	std::uint32_t id_;
	PermanentConnection::StreamInfo& streamInfo_;
	std::shared_ptr<PermanentConnection> connection_;
	ReadHandler readHandler_;
	WriteHandler writeHandler_;
	PeerID localPeerID_, remotePeerID_;
};