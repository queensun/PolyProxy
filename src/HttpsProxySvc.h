#pragma once
#include "ProxySvc.h"

class HttpsProxySvc
	: public ProxySvc
{
public:
	void createSession(std::unique_ptr<Stream> stream);

};