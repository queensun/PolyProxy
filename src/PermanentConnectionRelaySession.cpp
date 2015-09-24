#include "PermanentConnectionRelaySession.h"
#include "PermanentConnection.h"
#include "RelaySession.h"

void PermanentConnectionRelaySession::go()
{
	auto _ = shared_from_this();
	boost::asio::spawn(ios,
		[this, _](boost::asio::yield_context yield)
	{
		try
		{
			std::unique_ptr<Stream> stream2 = 
				connection_->open(virtualService_, yield);
			relay(std::move(stream_), std::move(stream2));
		}
		catch (boost::system::system_error& /*e*/)
		{
		}
	});
}