#pragma once

#include "Stream.h"
#include "static.h"

class TcpStream
	: public Stream
{
public:
	TcpStream(boost::asio::ip::tcp::socket &&socket)
		: socket_(std::move(socket))
	{}
	TcpStream(TcpStream &&o)
		: socket_(std::move(o.socket_))
	{}

	virtual void readSome(const MutableBuffers& buffers, const ReadHandler& handler);
	virtual void writeSome(const ConstBuffers& buffers, const WriteHandler& handler);
	virtual boost::system::error_code eof(boost::asio::yield_context yield);
	//virtual void cancel();
	virtual boost::system::error_code close(boost::asio::yield_context yield);
	virtual void close();
	virtual PeerID localPeerID() const;
	virtual PeerID remotePeerID() const;

protected:
	boost::asio::ip::tcp::socket socket_;
};

class LANTcpStream
	: public TcpStream
{
public:
	LANTcpStream(boost::asio::ip::tcp::socket &&socket)
		: TcpStream(std::move(socket))
	{}
	LANTcpStream(LANTcpStream&& lANTcpStream)
		: TcpStream(std::move(lANTcpStream))
	{}
	LANTcpStream(TcpStream&& tcpStream)
		: TcpStream(std::move(tcpStream))
	{}

	virtual PeerID localPeerID() const
	{
		return PeerID();
	}
};
