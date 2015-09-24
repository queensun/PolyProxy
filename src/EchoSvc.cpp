#include "EchoSvc.h"

void EchoSvc::createSession(std::unique_ptr<Stream> stream)
{
	std::shared_ptr<Session> session(new EchoSession(std::move(stream)));
	session->go();
}

void EchoSession::go()
{
	auto _ = shared_from_this();
	boost::asio::spawn(ios,
		[this, _](boost::asio::yield_context yield)
	{
		try
		{
			while (true)
			{
				auto data = stream_->readSome(yield);
				stream_->write(data, yield);
			}
		}
		catch (boost::system::system_error& /*e*/)
		{
		}
	});
}