#include "PermanentConnectionStream.h"

void PermanentConnectionStream::readSome(
	const MutableBuffers& buffers, const ReadHandler& handler)
{
	readHandler_=std::move(handler);
	connection_->read(id_, streamInfo_, buffers, 
		[this](const boost::system::error_code& ec, std::size_t size)
	{
		if (readHandler_)
		{
			ReadHandler handler;
			std::swap(readHandler_, handler);
			handler(ec, size);
		}
	});
}

void PermanentConnectionStream::writeSome(
	const ConstBuffers& buffers, const WriteHandler& handler)
{
	writeHandler_ = std::move(handler);
	connection_->write(
		PermanentConnection::Command::transfer,
		id_,
		buffers,
		[this](const boost::system::error_code& ec, std::size_t size)
	{
		std::cout << "written: " << size << std::endl;
		if (writeHandler_)
		{
			WriteHandler handler;
			std::swap(writeHandler_, handler);
			handler(ec, size);
		}
	});
}

boost::system::error_code PermanentConnectionStream::eof(
	boost::asio::yield_context yield)
{
	try
	{
		connection_->write(
			PermanentConnection::Command::eof,
			id_,
			boost::asio::null_buffers(),
			yield
			);
	}
	catch (boost::system::system_error& e)
	{
		return e.code();
	}

	return boost::system::error_code();
}
//
//void PermanentConnectionStream::cancel()
//{
//	if (readHandler_)
//	{
//		ReadHandler handler;
//		std::swap(readHandler_, handler);
//		handler(boost::system::error_code(boost::asio::error::operation_aborted), 0);
//	}
//
//	if (writeHandler_)
//	{
//		WriteHandler handler;
//		std::swap(writeHandler_, handler);
//		handler(boost::system::error_code(boost::asio::error::operation_aborted), 0);
//	}
//}

boost::system::error_code PermanentConnectionStream::close(boost::asio::yield_context yield)
{
	if (!closed_)
	{
		try
		{
			closed_ = true;
			connection_->write(
				PermanentConnection::Command::close,
				id_,
				boost::asio::null_buffers(),
				yield
				);
		}
		catch (boost::system::system_error& e)
		{
			return e.code();
		}
	}
	return boost::system::error_code();
}

PeerID PermanentConnectionStream::localPeerID() const
{
	return localPeerID_;
}

PeerID PermanentConnectionStream::remotePeerID() const
{
	return remotePeerID_;
}

PermanentConnectionStream::~PermanentConnectionStream()
{
	if (!closed_)
	{
		connection_->write(
			PermanentConnection::Command::reset,
			id_,
			boost::asio::null_buffers(),
			[](const boost::system::error_code&, std::size_t)
		{
		});
	}
	connection_->streamEnded(id_);
}
