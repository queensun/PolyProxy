#include "ObfsSession.h"
#include "ObfsStream.h"

void ObfsSession::go()
{
	auto _ = shared_from_this();
	boost::asio::spawn(ios,
		[this, _](boost::asio::yield_context yield)
	{
		try
		{
			auto ctxes = svc_->key().exchange(stream_.get(), yield);
			assert(dynamic_cast<TcpStream*>(stream_.get()));
			std::unique_ptr<Stream> stream = std::unique_ptr<Stream>(new ObfsStream<TcpStream>(
				std::move(static_cast<TcpStream&>(*stream_)),
				Obfs(std::move(ctxes.first)), Obfs(std::move(ctxes.second)))
				);
			svc_->passStream(std::move(stream), yield);
		}
		catch (boost::system::system_error&)
		{
		}		
	});
}