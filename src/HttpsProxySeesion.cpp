#include "RelaySession.h"
#include "HttpsProxySeesion.h"
#include "http.h"

void HttpsProxySeesion::go()
{
	auto _ = shared_from_this();
	boost::asio::spawn(ios,
		[this, _](boost::asio::yield_context yield)
	{
		try
		{
			HttpRequestMessage msg = readHttpRequestMessage(stream_.get(), yield);
			if (msg.requestLine.method == "CONNECT")
			{
				std::unique_ptr<Stream> stream2;
				bool connectionAllowed;
				boost::system::error_code ec, convertEc;
				
				boost::asio::ip::address address =
					boost::asio::ip::address::from_string(msg.requestLine.uri.host, convertEc);

				if (!convertEc)
				{
					if (connectionAllowed = (*acl_)(stream_->remotePeerID(), address, msg.requestLine.uri.port))
					{
						try
						{
							stream2 = upStreamProxy_->connect(address, msg.requestLine.uri.port, yield);
						}
						catch (boost::system::system_error& e)
						{
							ec = e.code();
						}
					}
				}
				else
				{
					if (connectionAllowed = (*acl_)(stream_->remotePeerID(), msg.requestLine.uri.host, msg.requestLine.uri.port))
					{
						try
						{
							stream2 = upStreamProxy_->connect(msg.requestLine.uri.host, msg.requestLine.uri.port, yield);
						}
						catch (boost::system::system_error& e)
						{
							ec = e.code();
						}
					}
				}

				if (connectionAllowed)
				{
					if (!ec)
					{
						writeResponse(200, "Connection established", yield);
						relay(std::move(stream_), std::move(stream2));
					}
				}
				else	//connection not allowed
				{
					writeResponse(403, "Forbidden", yield);
				}
			}
		}
		catch (InvalidProtocol&)
		{
		}
		catch (HttpMessage::InvalidProtocol&)
		{
		}
		catch (boost::system::system_error& /*e*/)
		{
		}
	}, boost::coroutines::attributes(1024 * 1024));
}

void HttpsProxySeesion::writeResponse(
	unsigned int statusCode, const std::string& statusText, boost::asio::yield_context yield)
{
	std::stringstream ss;
	ss
		<< "HTTP/1.1 "
		<< statusCode << ' '
		<< statusText << "\r\n\r\n";

	stream_->write(
		boost::asio::buffer(ss.str().data(), ss.str().size()), yield);
}
