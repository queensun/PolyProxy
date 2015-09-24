#include "static.h"
#include "Socks5Svc.h"
#include "HttpsProxySvc.h"
#include "AllProxySvc.h"
#include "TcpEntry.h"
#include "HttpsUpStreamProxy.h"
#include "SslUpStreamProxy.h"

void createServices()
{
	std::shared_ptr<SslContext> ctx = std::make_shared<SslContext>(
		boost::asio::ssl::context::method::sslv23_client);
	ctx->load_verify_file("apca.crt");
	ctx->use_certificate_file("apclient.crt", SslContext::file_format::pem);
	ctx->use_private_key_file("apclient.key", SslContext::file_format::pem);
	ctx->set_verify_mode(SslContext::verify_peer);

	defaultUpStreamProxy.reset(new LANUpStreamProxy());
	std::unique_ptr<Entry> entry(new TcpEntry(1234));
	std::shared_ptr<Svc> svc(new AllProxySvc);

	std::shared_ptr<SslUpStreamProxy> sslUpStreamProxy(
		new HostSslUpStreamProxy(ctx, "APCA", defaultUpStreamProxy));
	sslUpStreamProxy->sni("APCA");
	sslUpStreamProxy->timeout(15000);

	std::shared_ptr<UpStreamProxy> upStreamProxy(new HttpsUpStreamProxy(createConnector(
		boost::asio::ip::address_v4::from_string("???"), 443, sslUpStreamProxy)));


	std::shared_ptr<Svc> socks5Svc(new Socks5Svc);
	std::static_pointer_cast<Socks5Svc>(socks5Svc)->upStreamProxy(upStreamProxy);
	std::shared_ptr<Svc> httpsProxySvc(new HttpsProxySvc);
	std::static_pointer_cast<HttpsProxySvc>(httpsProxySvc)->upStreamProxy(upStreamProxy);

	*std::static_pointer_cast<AllProxySvc>(svc)
		<< std::static_pointer_cast<Socks5Svc>(socks5Svc)
		<< std::static_pointer_cast<HttpsProxySvc>(httpsProxySvc)
		;

	svc->insertEntry(std::move(entry));
	socks5Svc->start();
	httpsProxySvc->start();
	svc->start();

}