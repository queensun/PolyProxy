#include "http.h"
#include <cstdlib> 

namespace
{
	void readHttpHeaders(Stream* stream, HttpMessage& msg, boost::asio::yield_context yield)
	{
		while (true)
		{
			auto _ = stream->readLine(yield);
			ConstStringFromBuffers line(_);
			msg.source.headers.append(std::move(_));
			line = line.chomp();

			short c = line.empty()?-1:line.front();
			
			if (!msg.headers.empty())
			{
				auto &text = msg.headers.back().text;
				auto column = std::find(text.begin(), text.end(), ':');
				if (column == text.end())
				{
					if (line.empty())
					{
						break;
					}
					msg.headers.push_back(HttpMessage::Header());
					msg.headers.back().text = line;
				}
				else if (c == ' ' || c == '\t' || c == '\x09')
				{
					assert(!line.empty());
					msg.headers.back().text = msg.headers.back().text + line;
				}
				else
				{
					msg.headers.back().name = text.substr(text.begin(), column);
					msg.headers.back().value = text.substr(++column, text.end());

					if (line.empty())
					{
						break;
					}
					msg.headers.push_back(HttpMessage::Header());
					msg.headers.back().text = line;
				}
			}
			else
			{
				if (line.empty())
				{
					break;
				}
				msg.headers.push_back(HttpMessage::Header());
				msg.headers.back().text = line;
			}			
		}
	}
}

HttpRequestMessage readHttpRequestMessage(Stream* stream, boost::asio::yield_context yield)
{
	HttpRequestMessage ret;
	while (true)
	{
		ret.source.firstLine = stream->readLine(yield);
		ConstStringFromBuffers line(ret.source.firstLine);
		line = line.chomp();
		if (!line.empty())
		{
			auto methodEnd=std::find(line.begin(), line.end(), ' ');
			if (methodEnd == line.end())
				throw HttpMessage::InvalidProtocol();

			ret.requestLine.method = constStringCast<std::string>(line.substr(line.begin(), methodEnd));

			auto uriBegin = methodEnd;
			++uriBegin;
			auto uriEnd = std::find(uriBegin, line.end(), ' ');
			std::string uri = constStringCast<std::string>(line.substr(uriBegin, uriEnd));
			std::string::size_type pos = 0;
			std::string::size_type protocolEnd=uri.find("://");
			if (protocolEnd != std::string::npos)
			{
				ret.requestLine.uri.protocol = uri.substr(0, protocolEnd);
				std::transform(ret.requestLine.uri.protocol.begin(), ret.requestLine.uri.protocol.end(), 
					ret.requestLine.uri.protocol.begin(), static_cast<int (*) (int)>(&std::tolower));
				pos = protocolEnd + 3;
			}

			if (pos != 0 || uri[pos] != '/')
			{
				std::string::size_type hostEnd = uri.find('/', pos);
				if (hostEnd == std::string::npos)
				{
					ret.requestLine.uri.host = uri.substr(pos);
				}
				else
				{
					ret.requestLine.uri.host = uri.substr(pos, hostEnd-pos);
				}
				pos = hostEnd;

				{
					std::string::size_type column = ret.requestLine.uri.host.find(':');
					if (column != std::string::npos)
					{
						std::string sPort = ret.requestLine.uri.host.substr(column + 1);
						ret.requestLine.uri.port = std::atoi(sPort.c_str());
						ret.requestLine.uri.host.resize(column);
					}
					else
					{
						if (ret.requestLine.uri.protocol == "http")
						{
							ret.requestLine.uri.port = 80;
						}
						else if (ret.requestLine.uri.protocol == "https")
						{
							ret.requestLine.uri.port = 443;
						}
					}
				}

				std::transform(ret.requestLine.uri.host.begin(), ret.requestLine.uri.host.end(),
					ret.requestLine.uri.host.begin(), static_cast<int (*) (int)>(&std::tolower));
			}
			else
			{
			}
			
			if (pos != std::string::npos)
			{
				std::string::size_type pathEnd = uri.find('?', pos);
				ret.requestLine.uri.path = uri.substr(pos, pathEnd);
				if (pathEnd != std::string::npos)
				{
					ret.requestLine.uri.query = uri.substr(pathEnd + 1);
				}
			}
			else if (!ret.requestLine.uri.protocol.empty())
			{
				ret.requestLine.uri.path = "/";
			}

			auto versionBegin = uriEnd;
			if (*++versionBegin != 'H'
				|| *++versionBegin != 'T'
				|| *++versionBegin != 'T'
				|| *++versionBegin != 'P'
				|| *++versionBegin != '/'
				)
				throw HttpMessage::InvalidProtocol();
			++versionBegin;
			
			auto versionEnd = std::find(versionBegin, line.end(), '.');
			if (versionEnd == line.end())
				throw HttpMessage::InvalidProtocol();
			
			ret.requestLine.versionMajor = std::atoi(
				constStringCast<std::string>(line.substr(versionBegin, versionEnd)).c_str());

			versionBegin = versionEnd;
			++versionBegin;

			ret.requestLine.versionMinor = std::atoi(
				constStringCast<std::string>(line.substr(versionBegin)).c_str());

			break;
		}
	}

	readHttpHeaders(stream, ret, yield);
	return ret;
}

HttpResponseMessage readHttpResponseMessage(Stream* stream, boost::asio::yield_context yield)
{
	HttpResponseMessage ret;
	while (true)
	{
		ret.source.firstLine = stream->readLine(yield);
		ConstStringFromBuffers line(ret.source.firstLine);
		line = line.chomp();
		if (!line.empty())
		{
			if (line.size() < 6)
			{
				throw HttpMessage::InvalidProtocol();
			}

			auto versionBegin = line.begin();
			if (*versionBegin != 'H'
				|| *++versionBegin != 'T'
				|| *++versionBegin != 'T'
				|| *++versionBegin != 'P'
				|| *++versionBegin != '/'
				)
				throw HttpMessage::InvalidProtocol();
			++versionBegin;

			auto versionEnd = std::find(versionBegin, line.end(), '.');
			if (versionEnd == line.end())
				throw HttpMessage::InvalidProtocol();
			ret.responseLine.versionMajor = std::atoi(
				constStringCast<std::string>(line.substr(versionBegin, versionEnd)).c_str());

			versionBegin = versionEnd;
			++versionBegin;
			versionEnd = std::find(versionBegin, line.end(), ' ');
			if (versionEnd == line.end())
				throw HttpMessage::InvalidProtocol();
			ret.responseLine.versionMinor = std::atoi(
				constStringCast<std::string>(line.substr(versionBegin, versionEnd)).c_str());


			auto codeBegin = versionEnd;
			++codeBegin;
			auto codeEnd = std::find(codeBegin, line.end(), ' ');
			if (codeEnd == line.end())
				throw HttpMessage::InvalidProtocol();
			ret.responseLine.code = std::atoi(
				constStringCast<std::string>(line.substr(codeBegin, codeEnd)).c_str());

			auto textBegin = codeEnd;
			++textBegin;
			ret.responseLine.text =
				constStringCast<std::string>(line.substr(textBegin, line.end())).c_str();

			break;
		}
	}

	readHttpHeaders(stream, ret, yield);
	return ret;
}