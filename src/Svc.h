#pragma once
#include "static.h"
#include "Stream.h"
#include "Entry.h"

#include <boost/ptr_container/ptr_set.hpp>

class Svc
	: public std::enable_shared_from_this<Svc>
{
public:
	using EntryPtr = std::unique_ptr <Entry> ;
	inline Entry* insertEntry(std::unique_ptr<Entry> entry)
	{
		auto _ = shared_from_this();

		Entry* p = entry.release();
		entries_.insert(p);
		p->start();
		if (
			status() == Status::starting ||
			status() == Status::running ||
			status() == Status::paused
			)
		{
			linkEntry(*p);
		}
		boost::asio::spawn(ios,
			[_, this, p](boost::asio::yield_context yield)
		{
			p->resume(yield);
		}
		);
		return p;
	}
	inline void eraseEntry(Entry* entry)
	{
		auto it = entries_.find(*entry);
		if (it == entries_.end())
		{
			throw ElementAbsentExcept();
		}

		entries_.release(it).release();
		std::shared_ptr<Entry> zombie(entry);

		boost::asio::spawn(ios,
			[zombie](boost::asio::yield_context yield)
		{
			zombie->stop(yield);
		}
		);
	}

	inline void start(bool running = true)
	{
		for (Entry& entry : entries_)
		{
			entry.start();
		}

		if (status_ == Status::created)
		{
			status_ = Status::starting;
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
	
	virtual ~Svc()
	{}

protected:
	using Entries = boost::ptr_set<Entry>;
	Entries entries_;

	virtual void createSession(std::unique_ptr<Stream> stream) = 0;
	void linkEntry(Entry& entry)
	{
		auto _ = shared_from_this();
		boost::asio::spawn(ios,
			[&entry, this, _](boost::asio::yield_context yield)
		{
			try
			{
				while (true)
				{
					std::unique_ptr<Stream> stream = entry.accept(yield);
					if (!stream) break;
					if (status_ == Svc::Status::running)
					{
						createSession(std::move(stream));
					}
				}
			}
			catch (boost::system::system_error&)
			{
			}
			if (entries_.find(entry) != entries_.end())
			{
				eraseEntry(&entry);
			}
		}, boost::coroutines::attributes(1024 * 1024));
	}

	virtual void doStart()
	{
		for (Entry& entry : entries_)
		{
			linkEntry(entry);
		}
	}
	virtual void doStop()
	{}
	virtual void doPause()
	{}
	virtual void doResume()
	{}

	Status status_ = Status::created;
};