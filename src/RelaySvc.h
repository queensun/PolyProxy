#pragma once
#include "ProxySvc.h"
#include "UpStreamProxy.h"

class RelaySvc
	: public ProxySvc
{
public:
	RelaySvc(std::unique_ptr<Connector> connector)
		:connector_(std::move(connector))
	{}

	template <typename... T>
	RelaySvc(const T&... t)
		: RelaySvc(createConnector(t...))
	{}

	void createSession(std::unique_ptr<Stream> stream);

private:
	std::shared_ptr<Connector> connector_;
};