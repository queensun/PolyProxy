#pragma once
#include "ProxySvc.h"

class PermanentConnection;
class PermanentConnectionRelaySvc
	: public ProxySvc
{
public:
	PermanentConnectionRelaySvc(
		const std::string& virtualService)
		: virtualService_(virtualService)
	{}
	virtual void createSession(std::unique_ptr<Stream> stream);
	inline std::shared_ptr<PermanentConnection> connection() const
	{
		return connection_;
	}
	inline void connection(std::shared_ptr<PermanentConnection> connection)
	{
		connection_ = connection;
	}

protected:
	std::string virtualService_;
	std::shared_ptr<PermanentConnection> connection_;
};