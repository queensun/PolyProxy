#pragma once
#include "static.h"
#include "Host.h"

class HostPool;

struct HostStatistics
{
	HostStatistics(
		std::shared_ptr<HostPool> pool,
		const Host& host)
		: pool_(pool)
		, host(host)
	{}
	~HostStatistics();
	void onConnectFailed();

	unsigned int connectSuccess = 0;
	unsigned int connectDealy = 0;
	std::size_t  bytesSent = 0;
	std::size_t  bytesReceived = 0;
	unsigned int sendTime = 0;
	unsigned int receiveTime = 0;
	std::size_t  totalBytesSent = 0;
	std::size_t  totalBytesReceived = 0;
	unsigned int totalSendTime = 0;
	unsigned int totalReceiveTime = 0;
	unsigned int connectFailed = 0;
	Host host;

	inline boost::optional<std::size_t> sendSpeed() const
	{
		if (sendTime)
		{
			return bytesSent * 1000 / sendTime;
		}
		else
		{
			return boost::optional<std::size_t>();
		}
	}

	inline boost::optional<std::size_t> receiveSpeed() const
	{
		if (receiveTime)
		{
			return bytesReceived * 1000 / receiveTime;
		}
		else
		{
			return boost::optional<std::size_t>();
		}
	}

protected:
	std::shared_ptr<HostPool> pool_;
};
