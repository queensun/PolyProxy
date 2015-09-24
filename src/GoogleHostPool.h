#pragma once

#include "HostPool.h"

class GoogleHostPool
	: public HostPool
{
public:
	virtual void onConnectFailed(const Host& host);
	void add(std::shared_ptr<HostStatistics> host);

private:
	std::map<Host, std::shared_ptr<HostStatistics> > holding_;
};