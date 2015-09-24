#include "GoogleHostPool.h"

void GoogleHostPool::onConnectFailed(const Host& host)
{
	holding_.erase(host);
	HostPool::erase(host);
}

void GoogleHostPool::add(std::shared_ptr<HostStatistics> host)
{
	holding_.insert(std::make_pair(host->host, host));
}
