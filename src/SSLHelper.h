#pragma once
#include "static.h"

std::vector<X509*> toX509s(const char* str);
std::vector<X509*> toX509s(std::istream& is);
std::vector<X509*> toX509s(BIO* bio);
X509* toX509(const char* str);
X509* toX509(std::istream& is);
X509* toX509(BIO* bio);
EVP_PKEY* toPKey(const char* str);
EVP_PKEY* toPKey(std::istream& is);
EVP_PKEY* toPKey(BIO* bio);

struct CryptoFileExcept
	: public std::runtime_error
{
	CryptoFileExcept()
		: std::runtime_error("invalid crypto file")
	{}
	using std::runtime_error::runtime_error;
};
