#pragma once

#include "static.h"
#include "Stream.h"
#include <memory>
class Session
	: public std::enable_shared_from_this<Session>
{
public:
	Session(std::unique_ptr<Stream>&& stream)
		: stream_(std::move(stream))
	{}
	virtual ~Session()
	{}
	virtual void go() = 0;

protected:
	std::unique_ptr<Stream> stream_;
};