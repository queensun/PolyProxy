#include "PermanentConnectionConnector.h"

std::unique_ptr<Stream> PermanentConnectionConnector::operator()(boost::asio::yield_context yield)
{
	return connection_->open(virtualService_, yield);
}