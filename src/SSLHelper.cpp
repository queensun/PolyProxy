#include "SSLHelper.h"

std::vector<X509*> toX509s(const char* str)
{
	BIO *bio = BIO_new(BIO_s_mem());
	if (!bio)
	{
		throw std::bad_alloc();
	}
	BOOST_SCOPE_EXIT(bio)
	{
		::BIO_free(bio);
	} BOOST_SCOPE_EXIT_END;

	BIO_puts(bio, str);
	return toX509s(bio);
}

std::vector<X509*> toX509s(std::istream& is)
{
	std::string str((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
	return toX509s(str.c_str());
}

std::vector<X509*> toX509s(BIO* bio)
{
	std::vector<X509*> ret;
	while (X509* x509 = ::PEM_read_bio_X509(bio, 0, 0, 0))
	{
		ret.push_back(x509);
	}
	return std::move(ret);
}

X509* toX509(const char* str)
{
	BIO *bio = BIO_new(BIO_s_mem());
	if (!bio)
	{
		throw std::bad_alloc();
	}
	BOOST_SCOPE_EXIT(bio)
	{
		::BIO_free(bio);
	} BOOST_SCOPE_EXIT_END;

	BIO_puts(bio, str);
	return toX509(bio);
}

X509* toX509(std::istream& is)
{
	std::string str((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
	return toX509(str.c_str());
}

X509* toX509(BIO* bio)
{
	X509* ret=::PEM_read_bio_X509(bio, 0, 0, 0);
	if (ret == 0)
	{
		throw CryptoFileExcept();
	}
	return ret;
}

EVP_PKEY* toPKey(const char* str)
{
	BIO *bio = BIO_new(BIO_s_mem());
	if (!bio)
	{
		throw std::bad_alloc();
	}
	BOOST_SCOPE_EXIT(bio)
	{
		::BIO_free(bio);
	} BOOST_SCOPE_EXIT_END;

	BIO_puts(bio, str);
	return toPKey(bio);
}

EVP_PKEY* toPKey(std::istream& is)
{
	std::string str((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
	return toPKey(str.c_str());
}

EVP_PKEY* toPKey(BIO* bio)
{
	EVP_PKEY* ret = ::PEM_read_bio_PrivateKey(bio, 0, 0, 0);
	if (ret == 0)
	{
		throw CryptoFileExcept();
	}
	return ret;
}