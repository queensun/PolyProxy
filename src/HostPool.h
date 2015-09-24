#pragma once

#include "WeakPtrMap.h"
#include "Host.h"
#include "HostStatistics.h"

class HostPool
	: public WeakPtrMap < Host, HostStatistics, HostPool >
{
public:
	virtual ~HostPool(){}

	virtual void onConnectFailed(const Host& host)
	{}
};