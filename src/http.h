#pragma once
#include "static.h"
#include "Buffers.h"
#include "Stream.h"

struct HttpMessage
{
	struct Source
	{
		MutableOwnedBuffers firstLine;
		MutableOwnedBuffers headers;
	} source;


	struct Header
	{
		ConstStringFromBuffers name;
		ConstStringFromBuffers value;
		ConstStringFromBuffers text;
	};

	using Headers = std::vector < Header > ;

	Headers headers;

	class InvalidProtocol
	{};
};

struct HttpRequestMessage
	: public HttpMessage
{
	struct RequestLine
	{
		std::string	method;
		struct Uri
		{
			std::string			protocol;
			std::string			host;
			unsigned short		port = 0;
			std::string			path;
			std::string			query;
		} uri;
		unsigned int versionMajor;
		unsigned int versionMinor;
	} requestLine;
};

struct HttpResponseMessage
	: public HttpMessage
{
	struct RequestLine
	{
		unsigned int versionMajor;
		unsigned int versionMinor;

		unsigned int code;
		std::string text;
	} responseLine;
};

HttpRequestMessage readHttpRequestMessage(Stream*, boost::asio::yield_context yield);
HttpResponseMessage readHttpResponseMessage(Stream*, boost::asio::yield_context yield);