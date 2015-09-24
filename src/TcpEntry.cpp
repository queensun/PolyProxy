#include "TcpEntry.h"
#include "TcpStream.h"

void TcpEntry::doStart()
{
	acceptor_.open(address_.is_v4() ? boost::asio::ip::tcp::v4() : boost::asio::ip::tcp::v6());
	acceptor_.set_option(reuse_);

	acceptor_.bind(boost::asio::ip::tcp::endpoint(address_, port_));
	acceptor_.listen();
}

std::unique_ptr<Stream> TcpEntry::accept(boost::asio::yield_context yield)
{
	std::unique_ptr<Stream> ret;

	do 
	{
		boost::asio::ip::tcp::socket socket(ios);
		boost::system::error_code ec;
		acceptor_.async_accept(socket, yield[ec]);
		if (ec)
		{
			return nullptr;
		}
		ret.reset(new TcpStream(std::move(socket)));
	} while (status() != Status::running);

	return ret;
}

void TcpEntry::doStop(boost::asio::yield_context yield)
{
	acceptor_.cancel();
}

