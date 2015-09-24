#include "static.h"
#include "TcpEntry.h"
#include "UpStreamProxy.h"
#include "ObfsUpStreamProxy.h"
#include "PermanentConnection.h"
#include "PermanentConnectionRelaySvc.h"
#include "PermanentConnectionConnector.h"
#include "ObfsSvc.h"
#include "RelaySvc.h"
#include "SslContext.h"
#include "SslUpStreamProxy.h"
#include "AllProxySvc.h"
#include "HttpsUpStreamProxy.h"
#include "SSLHelper.h"

void createServices()
{
	std::shared_ptr < UpStreamProxy > obfsUpStreamProxy(
		new ObfsUpStreamProxy("All men are created equal."));
	obfsUpStreamProxy->timeout(15000);
	
	std::unique_ptr<Entry> entry(new TcpEntry(6001));
	std::shared_ptr<Svc> svc(
		new RelaySvc(createConnector(boost::asio::ip::address_v4::from_string("127.0.0.1"), 6002, obfsUpStreamProxy)));
	svc->insertEntry(std::move(entry));
	svc->start();
}