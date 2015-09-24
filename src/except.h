#pragma once

#include <stdexcept>
#include <iostream>
#include <boost/system/system_error.hpp>

class InvalidAssignmentExcept
	: public std::runtime_error
{
public:
	InvalidAssignmentExcept()
		: std::runtime_error("invalid assignment")
	{}
	using std::runtime_error::runtime_error;
};

class ElementAbsentExcept
	: public std::runtime_error
{
public:
	ElementAbsentExcept()
		: std::runtime_error("element not found")
	{}
	using std::runtime_error::runtime_error;
};

enum class NetworkError
{
	generalError=0x10000,
	invalidCertificate,
	insufficientResources,
	protocolError
};

class NetworkErrorCat
	: public boost::system::error_category
{
public:
	virtual const char * name() const NOEXCEPT
	{
		return "NetworkErrorCat";
	}

	virtual std::string message(int ev) const
	{
		return "NetworkErrorCat";
	}

	static void throwError(NetworkError e)
	{
		static NetworkErrorCat errCat;		
		throw boost::system::system_error(boost::system::error_code(static_cast<int>(e), errCat));
	}
};