#include "RelaySession.h"

void RelaySession::go()
{
	auto _ = shared_from_this();
	boost::asio::spawn(ios,
		[this, _](boost::asio::yield_context yield)
	{
		try
		{
			std::unique_ptr<Stream> stream2 = (*connector_)(yield);
			relay(std::move(stream_), std::move(stream2));
		}
		catch (boost::system::system_error& /*e*/)
		{
		}
	});
}

void relay(std::shared_ptr<Stream> stream1, std::shared_ptr<Stream> stream2)
{
	auto func=
		[](std::shared_ptr<Stream> rs, std::shared_ptr<Stream> ws, 
		boost::asio::yield_context yield)
	{
		bool eof = false;
		bool error = false;
		try
		{
			while (1)
			{
				auto data = rs->readSome(yield);
				ws->write(data, yield);
			}
		}
		catch (boost::system::system_error& e)
		{
			boost::system::error_code ec = e.code();
			if (ec == boost::asio::error::eof)
			{
				eof = true;
			}
			else
			{
				error = true;
			}
		}
		if (eof)
		{
			if (ws->eof(yield))
			{
				error = true;
			}
		}
		if (error)
		{
			rs->close(yield);
			ws->close(yield);
		}
	};

	boost::asio::spawn(ios,
		[stream1, stream2, func](boost::asio::yield_context yield)
	{
		func(stream1, stream2, yield);
	});
	boost::asio::spawn(ios,
		[stream1, stream2, func](boost::asio::yield_context yield)
	{
		func(stream2, stream1, yield);
	});
}