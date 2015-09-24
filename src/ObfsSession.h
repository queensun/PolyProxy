#pragma once

#include "static.h"
#include "ObfsSvc.h"
#include "Session.h"

class ObfsSession
	: public Session
{
public:
	ObfsSession(std::unique_ptr<Stream>&& stream, std::shared_ptr<ObfsSvc> svc)
		: Session(std::move(stream))
		, svc_(svc)
	{}

	void go();

protected:
	std::shared_ptr<ObfsSvc> svc_;
};