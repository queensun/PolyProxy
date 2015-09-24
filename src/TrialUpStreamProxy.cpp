#include "TrialUpStreamProxy.h"
#include "TrialStream.h"

std::unique_ptr<Stream> TrialUpStreamProxy::connect(
	const std::string& host, unsigned short port, boost::asio::yield_context yield)
{
	std::shared_ptr<HostStatistics> statistics = (*pool_)[host];
	auto begin = std::chrono::steady_clock::now();
	try
	{
		auto ret = nextLayer_->connect(host, port, yield);
		unsigned int elapsed =
			std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now() - begin).count();
		statistics->connectDealy =
			(statistics->connectDealy * statistics->connectSuccess + elapsed) /
			(statistics->connectSuccess + 1);
		++statistics->connectSuccess;

		return std::make_unique<TrialStream>(std::move(ret), statistics);
	}
	catch (boost::system::system_error&)
	{
		++statistics->connectFailed;
		statistics->onConnectFailed();
		throw;
	}
}

std::unique_ptr<Stream> TrialUpStreamProxy::connect(
	const boost::asio::ip::address& address, unsigned short port, boost::asio::yield_context yield)
{
	std::shared_ptr<HostStatistics> statistics = (*pool_)[address];
	auto begin = std::chrono::steady_clock::now();
	try
	{
		auto ret = nextLayer_->connect(address, port, yield);
		unsigned int elapsed =
			std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now() - begin).count();
		statistics->connectDealy =
			(statistics->connectDealy * statistics->connectSuccess + elapsed) /
			(statistics->connectSuccess + 1);
		++statistics->connectSuccess;

		return std::make_unique<TrialStream>(std::move(ret), statistics);
	}
	catch (boost::system::system_error&)
	{
		++statistics->connectFailed;
		statistics->onConnectFailed();
		throw;
	}
}