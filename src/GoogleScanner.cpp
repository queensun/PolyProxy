#include "GoogleScanner.h"
#include "http.h"

#include <boost/scope_exit.hpp>

void GoogleScanner::check(const boost::asio::ip::address_v4& ip, boost::asio::yield_context yield)
{
	auto statistics = (*hostPool_)[ip];
	if (valid(ip, "www.google.com", yield))
	{
		//hostPool_->add(statistics);
		std::cout << ip << "www.google.com" << std::endl;
	}
	else
		std::cout << "!" << std::endl;
	if (valid(ip, "accounts.google.com", yield))
	{
		std::cout << ip << "accounts.google.com" << std::endl;
	}
	else
		std::cout << "!" << std::endl;
	if (valid(ip, "images.google.com", yield))
	{
		std::cout << ip << "images.google.com" << std::endl;
	}
	else
		std::cout << "!" << std::endl;;
	if (valid(ip, "mail.google.com", yield))
	{
		std::cout << ip << "mail.google.com" << std::endl;
	}
	else
		std::cout << "!" << std::endl;;
	if (valid(ip, "code.google.com", yield))
	{
		std::cout << ip << "code.google.com" << std::endl;
	}
	else
		std::cout << "!" << std::endl;;
	if (valid(ip, "www.youtube.com", yield))
	{
		std::cout << ip << "www.youtube.com" << std::endl;
	}
	else
		std::cout << "!" << std::endl;;
	if (valid(ip, "www.appspot.com", yield))
	{
		std::cout << ip << "www.appspot.com" << std::endl;
	}
	else
		std::cout << "!" << std::endl;;

	//std::cout << ip
	//	<< ": " << statistics->connectDealy / statistics->connectSuccess
	//	<< " / " << (statistics->sendSpeed() ? * statistics->sendSpeed() : 0)
	//	<< " / " << (statistics->receiveSpeed() ? *statistics->receiveSpeed():0)
	//	<< std::endl;
}

bool GoogleScanner::valid(
	const boost::asio::ip::address_v4& ip,
	const std::string& hostname,
	boost::asio::yield_context yield)
{
	std::string request =
"GET / HTTP/1.1\r\n"
"Accept: text/html, application/xhtml+xml, */*\r\n"
"Accept-Language: en-GB\r\n"
"User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; Trident/7.0; rv:11.0) like Gecko\r\n"
"Host: " + hostname + "\r\n"
"Connection: Keep-Alive\r\n"
"\r\n"
		;

	try
	{
		std::unique_ptr<Stream> stream = upStreamProxy_->connect(ip, port_, yield);
		stream->write(boost::asio::buffer(request.data(), request.size()), yield);
		HttpResponseMessage response = readHttpResponseMessage(stream.get(), yield);
		if (response.responseLine.code == 200
			|| response.responseLine.code / 100 == 3)
		{
			return true;
		}
	}
	catch (HttpMessage::InvalidProtocol&)
	{
	}

	return false;
}
