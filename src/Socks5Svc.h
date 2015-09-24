#pragma once
#include "ProxySvc.h"

class Socks5Svc
	: public ProxySvc
{
public:
	void createSession(std::unique_ptr<Stream> stream);
};