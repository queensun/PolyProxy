#pragma once
#include "static.h"

class SslContext
	: public boost::asio::ssl::context
{
public:
	using Base = boost::asio::ssl::context;
	SslContext(method m = Base::method::sslv23)
		: Base(ios, m)
	{}
};