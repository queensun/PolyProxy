#include "UpStreamProxy.h"
#include "TcpStream.h"

std::shared_ptr<UpStreamProxy> defaultUpStreamProxy(new UpStreamProxy);

std::unique_ptr<Stream> UpStreamProxy::connect(
	const std::string& host, unsigned short port, boost::asio::yield_context yield)
{
	return connect(resolve(host, yield), port, yield);
}

std::unique_ptr<Stream> UpStreamProxy::connect(
	const boost::asio::ip::address& address, unsigned short port, 
	boost::asio::yield_context yield)
{
	boost::asio::steady_timer timer(ios);
	BOOST_SCOPE_EXIT(&timer){
		timer.cancel();
	}BOOST_SCOPE_EXIT_END
	boost::asio::ip::tcp::socket socket(ios);

	bool timedout = false;
	if (timeout_)
	{
		timer.expires_from_now(std::chrono::milliseconds(timeout_));
		timer.async_wait(
			[&socket, &timedout](const boost::system::error_code& ec)
		{
			if (ec)
				return;

			boost::system::error_code ignore;
			socket.close(ignore);
			timedout = true;
		}
			);
	}

	try
	{
		socket.async_connect(
			boost::asio::ip::tcp::endpoint(address, port),
			yield);
	}
	catch (boost::system::system_error& e)
	{
		if (timedout)
		{
			throw boost::system::system_error(
				boost::system::error_code(boost::asio::error::timed_out));
		}
		else
		{
			throw;
		}
	}
	return std::unique_ptr<Stream>(
		new TcpStream(std::move(socket)));
}

boost::asio::ip::address UpStreamProxy::resolve(
	const std::string& host, boost::asio::yield_context yield)
{
	boost::asio::ip::tcp::resolver::query query(host, "");
	boost::asio::ip::tcp::resolver resolver(ios);
	boost::asio::ip::tcp::resolver::iterator it = resolver.async_resolve(query, yield);

	if (it != boost::asio::ip::tcp::resolver::iterator())
	{
		boost::asio::ip::tcp::endpoint endpoint = *it;
		return endpoint.address();
	}

	throw boost::system::system_error(boost::asio::error::host_not_found);
}

std::string UpStreamProxy::resolve(
	const boost::asio::ip::address& address, boost::asio::yield_context yield)
{
	boost::asio::ip::tcp::endpoint endPoint;
	endPoint.address(address);
	boost::asio::ip::tcp::resolver resolver(ios);
	boost::asio::ip::tcp::resolver::iterator it = resolver.async_resolve(endPoint, yield);

	if (it != boost::asio::ip::tcp::resolver::iterator())
	{
		return it->host_name();
	}
	
	throw boost::system::system_error(boost::asio::error::host_not_found);
}

std::unique_ptr<Stream> LANUpStreamProxy::connect(
	const std::string& host, unsigned short port, boost::asio::yield_context yield)
{
	std::unique_ptr<TcpStream> tmp{
		static_cast<TcpStream*>(UpStreamProxy::connect(host, port, yield).release())
	};

	return std::unique_ptr<Stream>(new LANTcpStream(std::move(*tmp)));
}

std::unique_ptr<Stream> LANUpStreamProxy::connect(
	const boost::asio::ip::address& address, unsigned short port, boost::asio::yield_context yield)
{
	std::unique_ptr<TcpStream> tmp{ 
		static_cast<TcpStream*>(UpStreamProxy::connect(address, port, yield).release())
	};
	return std::unique_ptr<Stream>(new LANTcpStream(std::move(*tmp)));
}