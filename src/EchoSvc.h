#pragma once

#include "static.h"
#include "Svc.h"
#include "Session.h"

class EchoSession;

class EchoSvc
	: public Svc
{
protected:
	virtual void createSession(std::unique_ptr<Stream> stream);
};

class EchoSession
	: public Session
{
public:
	EchoSession(std::unique_ptr<Stream>&& stream)
		: Session(std::move(stream))
	{}

	void go();
};