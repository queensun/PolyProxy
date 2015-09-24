#pragma once
#include "static.h"

struct InterCoroutineVarEndedExcept
{
};

template <typename T>
class InterCoroutineVar
{
public:
	using List = std::list < T > ;

	inline void push(T&& v)
	{
		buf_.emplace_back(std::move(v));
		readyTimer_.cancel();
	}

	inline void end()
	{
		readyTimer_.cancel();
	}

	inline T pop(boost::asio::yield_context yield)
	{
		if (buf_.empty())
		{
			boost::system::error_code ec;
			readyTimer_.async_wait(yield[ec]);
		}

		if (buf_.empty())
		{
			throw InterCoroutineVarEndedExcept();
		}

		T t = std::move(buf_.front());
		buf_.pop_front();
		return std::move(t);
	}

	template <typename Cb>
	inline void pop(Cb cb)
	{
		if (!buf_.empty())
		{
			ios.post(std::bind(cb, std::ref(buf_)));
		}
		else
		{
			readyTimer_.async_wait(
				[cb, this](const boost::system::error_code&)
			{
				cb(buf_);
			});
		}
	}

private:
	List buf_;
	boost::asio::steady_timer readyTimer_{ ios, boost::asio::steady_timer::clock_type::duration::max() };
};