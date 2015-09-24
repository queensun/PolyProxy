#include "AllProxySession.h"

void AllProxySession::go()
{
	auto _ = shared_from_this();
	boost::asio::spawn(ios,
		[this, _](boost::asio::yield_context yield)
	{
		try
		{
			auto data = stream_->peak(1, yield);
			InnerEntry* innerEntry;
			char c = *boost::asio::buffer_cast<char const*>(data.front());
			if (c == 'C' &&
				(innerEntry = svc_->innerEntries_[static_cast<std::size_t>(AllProxySvc::InnerEntryEnum::https)]))
			{
				innerEntry->passStream(std::move(stream_), yield);
			}
			else if (c == 5 &&
				(innerEntry=svc_->innerEntries_[static_cast<std::size_t>(AllProxySvc::InnerEntryEnum::socks5)]))
			{
				innerEntry->passStream(std::move(stream_), yield);
			}
			else if (c == 4 &&
				(innerEntry = svc_->innerEntries_[static_cast<std::size_t>(AllProxySvc::InnerEntryEnum::socks4)]))
			{
				innerEntry->passStream(std::move(stream_), yield);
			}
			else if (c == 22 &&
				(innerEntry = svc_->innerEntries_[static_cast<std::size_t>(AllProxySvc::InnerEntryEnum::ssl)]))
			{
				innerEntry->passStream(std::move(stream_), yield);
			}
			else if ((c == 'G' || c=='P') &&
				(innerEntry = svc_->innerEntries_[static_cast<std::size_t>(AllProxySvc::InnerEntryEnum::http)]))
			{
				innerEntry->passStream(std::move(stream_), yield);
			}
			else if (innerEntry = svc_->innerEntries_[static_cast<std::size_t>(AllProxySvc::InnerEntryEnum::obfuscation)])
			{
				innerEntry->passStream(std::move(stream_), yield);
			}
		}
		catch (boost::system::system_error& /*e*/)
		{
		}
	});
}