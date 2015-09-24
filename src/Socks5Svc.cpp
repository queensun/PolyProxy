#include "Socks5Svc.h"
#include "Socks5Session.h"

void Socks5Svc::createSession(std::unique_ptr<Stream> stream)
{
	std::shared_ptr<Session> session(
		new Socks5Session(upStreamProxy(), acl(), std::move(stream)));
	session->go();
}