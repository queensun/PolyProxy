#include "static.h"
#include "Socks5Svc.h"
#include "HttpsProxySvc.h"
#include "AllProxySvc.h"
#include "TcpEntry.h"
#include "HttpsUpStreamProxy.h"
#include "SslUpStreamProxy.h"

void createServices();
//void test()
//{
//	std::shared_ptr<SslContext> scannerSslContext = std::make_shared<SslContext>();
//	scannerSslContext->set_verify_mode(SslContext::verify_peer);
//	scannerSslContext->load_verify_file("D:\\cmdlet\\curl-ca-bundle.crt");
//	std::shared_ptr<GoogleSslUpStreamProxy> googleSslUpStreamProxy =
//		std::make_shared<GoogleSslUpStreamProxy>(scannerSslContext, std::make_shared<UpStreamProxy>());
//	googleSslUpStreamProxy->timeout(1800);
//
//	std::shared_ptr<GoogleHostPool> googleHostPool = std::make_shared<GoogleHostPool>();
//	std::shared_ptr<GoogleScanner> scanner = 
//		std::make_shared<GoogleScanner>(googleSslUpStreamProxy, googleHostPool);
//	scanner->addRanges("64.18.0.0/20 64.233.160.0/19 66.102.0.0/20 66.249.80.0/20 72.14.192.0/18 74.125.0.0/16 173.194.0.0/16 207.126.144.0/20 209.85.128.0/17 216.58.192.0/19 216.239.32.0/19");
//	scanner->port(443);
//	scanner->concurrence(1);
//	scanner->go();
//	
//	std::shared_ptr<SslContext> ctx = std::make_shared<SslContext>(
//		boost::asio::ssl::context::method::sslv23_client);
//	ctx->load_verify_file("D:\\Program Files (x86)\\stunnel\\AP\\apca.crt");
//	ctx->use_certificate_file("D:\\Program Files (x86)\\stunnel\\AP\\apclient.crt", SslContext::file_format::pem);
//	ctx->use_private_key_file("D:\\Program Files (x86)\\stunnel\\AP\\apclient.key", SslContext::file_format::pem);
//	ctx->set_verify_mode(SslContext::verify_peer);
//
//	defaultUpStreamProxy.reset(new LANUpStreamProxy());
//	std::unique_ptr<Entry> entry(new TcpEntry(1234));
//	std::shared_ptr<Svc> svc(new AllProxySvc);
//
//	std::shared_ptr<SslUpStreamProxy> sslUpStreamProxy = std::make_shared<HostSslUpStreamProxy>(ctx, "APCA", defaultUpStreamProxy);
//	sslUpStreamProxy->sni("APCA");
//
//	std::shared_ptr<UpStreamProxy> upStreamProxy(new HttpsUpStreamProxy(createConnector(
//		boost::asio::ip::address_v4::from_string("52.10.9.197"), 443, sslUpStreamProxy)));
//
//	std::shared_ptr<Svc> socks5Svc(new Socks5Svc);
//	std::static_pointer_cast<Socks5Svc>(socks5Svc)->upStreamProxy(upStreamProxy);
//	std::shared_ptr<Svc> httpsProxySvc(new HttpsProxySvc);
//	std::static_pointer_cast<HttpsProxySvc>(httpsProxySvc)->upStreamProxy(upStreamProxy);
//
//	*std::static_pointer_cast<AllProxySvc>(svc) 
//		<< std::static_pointer_cast<Socks5Svc>(socks5Svc)
//		<< std::static_pointer_cast<HttpsProxySvc>(httpsProxySvc)
//		;
//
//	svc->insertEntry(std::move(entry));
//	socks5Svc->start();
//	httpsProxySvc->start();
//	svc->start();
//}

int Main(std::vector<std::string> args)
{
	//SSL_CTX_set_tlsext_servername_callback
	createServices();

	ios.run();
	return 0;
}