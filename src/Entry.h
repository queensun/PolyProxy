#pragma once
#include "static.h"
#include "Stream.h"

class Entry
{
public:
	virtual std::unique_ptr<Stream> accept(boost::asio::yield_context yield) = 0;
	inline void start()
	{
		if (status_ == Status::created)
		{
			status_ = Status::starting;
			doStart();
			status_ = Status::paused;
		}
	}
	inline void stop(boost::asio::yield_context yield)
	{
		if (status_ != Status::stopped)
		{
			status_ = Status::stopping;
			doStop(yield);
			status_ = Status::stopped;
		}
	}
	inline void pause(boost::asio::yield_context yield)
	{
		if (status_ == Status::running)
		{
			doPause(yield);
			status_ = Status::paused;
		}
	}
	inline void resume(boost::asio::yield_context yield)
	{
		if (status_ == Status::paused)
		{
			doResume(yield);
			status_ = Status::running;
		}
	}

	enum class Status
	{
		created,
		starting,
		running,
		paused,
		stopping,
		stopped
	};

	inline Status status() const
	{
		return status_;
	}
	virtual ~Entry()
	{}

protected:
	virtual void doStart() = 0;
	virtual void doStop(boost::asio::yield_context yield)
	{}
	virtual void doPause(boost::asio::yield_context yield)
	{}
	virtual void doResume(boost::asio::yield_context yield)
	{}

	Status status_ = Status::created;
};

inline bool operator<(const Entry& l, const Entry& r)
{
	return &l < &r;
}