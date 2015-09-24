#include "TcpStream.h"

void TcpStream::readSome(const MutableBuffers& buffers, const ReadHandler& handler)
{
	socket_.async_read_some(buffers, handler);
}

void TcpStream::writeSome(const ConstBuffers& buffers, const WriteHandler& handler)
{
	socket_.async_write_some(buffers, handler);
}


boost::system::error_code TcpStream::eof(boost::asio::yield_context yield)
{
	boost::system::error_code ret;
	return socket_.shutdown(
		boost::asio::ip::tcp::socket::shutdown_send, ret);
}

//void TcpStream::cancel()
//{
//	boost::system::error_code e;
//	socket_.cancel(e);
//}

boost::system::error_code TcpStream::close(boost::asio::yield_context yield)
{
	close();
	return boost::system::error_code();
}

void TcpStream::close()
{
	socket_.close();
}

PeerID TcpStream::localPeerID() const
{
	return PeerID{ socket_.local_endpoint() };
}

PeerID TcpStream::remotePeerID() const
{
	return PeerID{ socket_.remote_endpoint() };
}
