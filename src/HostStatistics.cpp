#include "HostStatistics.h"
#include "HostPool.h"

HostStatistics::~HostStatistics()
{
	pool_->erase(host);
}

void HostStatistics::onConnectFailed()
{
	pool_->onConnectFailed(host);
}