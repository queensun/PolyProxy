#include "SslUpStreamProxy.h"
#include "SslStream.h"

std::unique_ptr<Stream> SslUpStreamProxy::connect(
	const std::string& host, unsigned short port, boost::asio::yield_context yield)
{
	auto begin = std::chrono::steady_clock::now();
	parent_->timeout(timeout_);
	std::unique_ptr<Stream> stream = parent_->connect(host, port, yield);
	unsigned int elapsed = 
		std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now() - begin).count();
	int timeout = timeout_ ? timeout_ - elapsed : 0;
	if (timeout < 0)
	{
		throw boost::system::system_error(
			boost::system::error_code(boost::asio::error::timed_out));
	}

	std::unique_ptr<Stream> ret = connect(std::move(stream), timeout, yield);
	if (!check(host, ret->remotePeerID()))
		NetworkErrorCat::throwError(NetworkError::invalidCertificate);
	return std::move(ret);
}

std::unique_ptr<Stream> SslUpStreamProxy::connect(
	const boost::asio::ip::address& address, unsigned short port, boost::asio::yield_context yield)
{
	auto begin = std::chrono::steady_clock::now();
	parent_->timeout(timeout_);
	std::unique_ptr<Stream> stream = parent_->connect(address, port, yield);
	unsigned int elapsed =
		std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::steady_clock::now() - begin).count();
	int timeout = timeout_ ? timeout_ - elapsed : 0;
	if (timeout < 0)
	{
		throw boost::system::system_error(
			boost::system::error_code(boost::asio::error::timed_out));
	}

	std::unique_ptr<Stream> ret = connect(std::move(stream), timeout, yield);
	if (!check(address, ret->remotePeerID()))
		NetworkErrorCat::throwError(NetworkError::invalidCertificate);
	return std::move(ret);
}

std::unique_ptr<Stream> SslUpStreamProxy::connect(
	std::unique_ptr<Stream> stream, unsigned int timeout, boost::asio::yield_context yield)
{
	SslStream* sslStream;
	std::unique_ptr<Stream> ret(sslStream = new SslStream(std::move(stream), *context_));
	boost::asio::steady_timer timer(ios);
	BOOST_SCOPE_EXIT(&timer){
		timer.cancel();
	}BOOST_SCOPE_EXIT_END;
	bool timedout = false;
	if (timeout)
	{
		timer.expires_from_now(std::chrono::milliseconds(timeout));
		timer.async_wait(
			[sslStream, &timedout](const boost::system::error_code& ec)
		{
			if (ec)
				return;

			sslStream->close();
			timedout = true;
		}
		);
	}

	try
	{
		sslStream->clientHandShake(sni(), yield);
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

	if (timedout)
	{
		throw boost::system::system_error(
			boost::system::error_code(boost::asio::error::timed_out));
	}
	return std::move(ret);
}

boost::asio::ip::address SslUpStreamProxy::resolve(
	const std::string& host, boost::asio::yield_context yield)
{
	return parent_->resolve(host, yield);
}
std::string SslUpStreamProxy::resolve(
	const boost::asio::ip::address& address, boost::asio::yield_context yield)
{
	return parent_->resolve(address, yield);
}

bool SslUpStreamProxy::check(const std::string& hostname, const PeerID& peerID)
{
	return peerID.certificate.checkCommonNames(
		[&hostname](char const* certHostName, std::size_t certHostNameLen) -> bool
	{
		return boost::iequals(
			hostname,
			std::make_pair(certHostName, certHostName + certHostNameLen));
	});
}

bool SslUpStreamProxy::check(const boost::asio::ip::address& address, const PeerID& peerID)
{
	return peerID.certificate.checkCommonNames(address);
}

bool HostSslUpStreamProxy::check(const std::string& hostname, const PeerID& peerID)
{
	return SslUpStreamProxy::check(hostname_, peerID);
}

bool HostSslUpStreamProxy::check(const boost::asio::ip::address& address, const PeerID& peerID)
{
	return SslUpStreamProxy::check(hostname_, peerID);
}
