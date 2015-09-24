#include "HttpsProxySvc.h"
#include "HttpsProxySeesion.h"

void HttpsProxySvc::createSession(std::unique_ptr<Stream> stream)
{	
	std::shared_ptr<Session> session(
		new HttpsProxySeesion(upStreamProxy(), acl(), std::move(stream)));
	session->go();
}