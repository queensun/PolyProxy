#include "static.h"
#include "TcpEntry.h"
#include "UpStreamProxy.h"
#include "PermanentConnection.h"
#include "ObfsSvc.h"
#include "RelaySvc.h"
#include "Socks5Svc.h"
#include "SSLHelper.h"

void createServices()
{
	std::unique_ptr<Entry> entry(new TcpEntry(6002));
	ObfsSvc* obfsSvc;
	std::shared_ptr<Svc> obfsSvcAuto(obfsSvc=new ObfsSvc("All men are created equal."));
	obfsSvc->insertEntry(std::move(entry));

	std::shared_ptr<Svc> svc(
		new RelaySvc(createConnector(boost::asio::ip::address_v4::from_string("127.0.0.1"), 6003)));
	obfsSvc->link(svc);
	
	obfsSvc->start();
	svc->start();
}