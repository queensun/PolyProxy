#pragma once

#include "Entry.h"
#include "static.h"

class TcpEntry
	: public Entry
{
public:
	TcpEntry()
		:port_(0)
	{}
	TcpEntry(unsigned int port)
		:port_(port)
	{}
	inline void address(const boost::asio::ip::address& address)
	{
		if (status_ == Status::created)
		{
			address_ = address;
		}
		else
		{
			throw InvalidAssignmentExcept();
		}
	}
	inline const boost::asio::ip::address& address() const
	{
		return address_;
	}
	inline void reuse(bool reuse)
	{
		if (status_ == Status::created)
		{
			reuse_ = reuse;
		}
		else
		{
			throw InvalidAssignmentExcept();
		}
	}
	inline bool reuse() const
	{
		return reuse_.value();
	}
	inline void port(unsigned int port)
	{
		if (status_ == Status::created)
		{
			port_ = port;
		}
		else
		{
			throw InvalidAssignmentExcept();
		}
	}
	unsigned int port() const
	{
		return port_;
	}

	virtual std::unique_ptr<Stream> accept(boost::asio::yield_context yield);

protected:
	virtual void doStart();
	virtual void doStop(boost::asio::yield_context yield);

	boost::asio::ip::address address_ = boost::asio::ip::address_v4::loopback();
	unsigned int port_;
	boost::asio::socket_base::reuse_address reuse_{ false };
	boost::asio::ip::tcp::acceptor acceptor_{ ios };
};