#pragma once
#include <memory>

template <class Object>
class StatusObject
	: public std::enable_shared_from_this<Object>
{
public:
	inline void start(bool running = true)
	{
		if (status_ == Status::created)
		{
			status_ = Status::starting;
			auto _ = shared_from_this();
			doStart();
			status_ = Status::paused;
			if (running)
			{
				resume();
			}
		}
	}
	inline void stop()
	{
		if (status_ != Status::stopped)
		{
			status_ = Status::stopping;
			doStop();
			status_ = Status::stopped;
		}
	}
	inline void pause()
	{
		if (status_ == Status::running)
		{
			doPause();
			status_ = Status::paused;
		}
	}
	inline void resume()
	{
		if (status_ == Status::paused)
		{
			doResume();
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
	~StatusObject()
	{}

protected:
	virtual void doStart() = 0;
	virtual void doStop() = 0;
	virtual void doPause() = 0;
	virtual void doResume() = 0;

	Status status_ = Status::created;
};