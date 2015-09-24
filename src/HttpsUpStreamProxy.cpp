#include "HttpsUpStreamProxy.h"
#include "http.h"

std::unique_ptr<Stream> HttpsUpStreamProxy::connect(
	const std::string& host, unsigned short port, boost::asio::yield_context yield)
{
	std::unique_ptr<Stream> stream = (*connector_)(yield);

	{
		std::stringstream ss;
		ss << "CONNECT " << host << ':' << port << " HTTP/1.1\r\n\r\n";
		stream->write(
			boost::asio::buffer(ss.str().data(), ss.str().size()),
			yield);
	}

	HttpResponseMessage response = readHttpResponseMessage(stream.get(), yield);

	switch (response.responseLine.code)
	{
	case 200:
		return stream;
	case 403:
		throw boost::system::system_error(boost::asio::error::access_denied);
	default:
		NetworkErrorCat::throwError(NetworkError::generalError);
	}	
}

std::unique_ptr<Stream> HttpsUpStreamProxy::connect(
	const boost::asio::ip::address& address, unsigned short port, boost::asio::yield_context yield)
{
	return connect(address.to_string(), port, yield);
}
