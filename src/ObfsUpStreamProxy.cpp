#include "ObfsUpStreamProxy.h"
#include "ObfsStream.h"

std::unique_ptr<Stream> ObfsUpStreamProxy::connect(
	const std::string& host, unsigned short port, boost::asio::yield_context yield)
{
	std::unique_ptr<Stream> stream = UpStreamProxy::connect(host, port, yield);
	auto ctxes = obfsKey_.exchange(stream.get(), yield);
	TcpStream *tcpStream = static_cast<TcpStream*>(stream.get());
	std::unique_ptr<Stream> ret = std::unique_ptr<Stream>(new ObfsStream<TcpStream>(
		std::move(*tcpStream), Obfs(std::move(ctxes.first)), Obfs(std::move(ctxes.second)))
		);
	return std::move(ret);
}

std::unique_ptr<Stream> ObfsUpStreamProxy::connect(
	const boost::asio::ip::address& address, unsigned short port, boost::asio::yield_context yield)
{
	std::unique_ptr<Stream> stream = UpStreamProxy::connect(address, port, yield);
	auto ctxes = obfsKey_.exchange(stream.get(), yield);
	TcpStream *tcpStream = static_cast<TcpStream*>(stream.get());
	std::unique_ptr<Stream> ret = std::unique_ptr<Stream>(new ObfsStream<TcpStream>(
		std::move(*tcpStream), Obfs(std::move(ctxes.first)), Obfs(std::move(ctxes.second)))
		);
	return std::move(ret);
}

std::unique_ptr<Stream> LANObfsUpStreamProxy::connect(
	const std::string& host, unsigned short port, boost::asio::yield_context yield)
{
	std::unique_ptr<Stream> stream = LANUpStreamProxy::connect(host, port, yield);

	auto ctxes = obfsKey_.exchange(stream.get(), yield);
	LANTcpStream *lANtcpStream = static_cast<LANTcpStream*>(stream.get());
	std::unique_ptr<Stream> ret = std::unique_ptr<Stream>(new ObfsStream<LANTcpStream>(
		std::move(*lANtcpStream), Obfs(std::move(ctxes.first)), Obfs(std::move(ctxes.second)))
		);
	return std::move(ret);
}

std::unique_ptr<Stream> LANObfsUpStreamProxy::connect(
	const boost::asio::ip::address& address, unsigned short port, boost::asio::yield_context yield)
{
	std::unique_ptr<Stream> stream = LANUpStreamProxy::connect(address, port, yield);
	auto ctxes = obfsKey_.exchange(stream.get(), yield);
	LANTcpStream *lANtcpStream = static_cast<LANTcpStream*>(stream.get());
	std::unique_ptr<Stream> ret = std::unique_ptr<Stream>(new ObfsStream<LANTcpStream>(
		std::move(*lANtcpStream), Obfs(std::move(ctxes.first)), Obfs(std::move(ctxes.second)))
		);
	return std::move(ret);
}