#include "SiteScanner.h"
#include "SslUpStreamProxy.h"

void SiteScanner::go()
{
	auto _ = shared_from_this();
	for (; running_ < concurrence_; ++running_)
	{
		std::size_t instance = running_;
		boost::asio::spawn(ios,
			[this, instance, _](boost::asio::yield_context yield)
		{
			while (instance < concurrence_)
			{
				boost::asio::ip::address_v4 ip = ipPool_.random();
				try
				{
					std::unique_ptr<Stream> stream = upStreamProxy_->connect(ip, port_, yield);
					check(ip, yield);
				}
				catch (boost::system::system_error &)
				{
				}
			}
			--running_;
		});
	}
}
