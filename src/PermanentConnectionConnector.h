#pragma once
#include "UpStreamProxy.h"
#include "PermanentConnection.h"

class PermanentConnectionConnector
	: public Connector
{
public:
	PermanentConnectionConnector(
		std::shared_ptr<PermanentConnection> connection, const std::string& virtualService)
		: connection_(connection)
		, virtualService_(virtualService)
	{}
	virtual std::unique_ptr<Stream> operator()(boost::asio::yield_context yield);

protected:
	std::shared_ptr<PermanentConnection> connection_;
	std::string virtualService_;
};

inline std::unique_ptr<Connector> createConnector(
	std::shared_ptr<PermanentConnection> connection, const std::string& virtualService)
{
	return std::unique_ptr<Connector>(new PermanentConnectionConnector(connection, virtualService));
}