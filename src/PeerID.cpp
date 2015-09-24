#include "PeerID.h"

bool PeerID::Certificate::checkWebHostName(
	const std::string& hostname) const
{
	auto func=
		[&hostname](char const* certHostName, std::size_t certHostNameLen) -> bool
	{
		if (boost::iequals(
			hostname, 
			std::make_pair(certHostName, certHostName + certHostNameLen)))
			return true;

		/* Wildcard match? */
		if (certHostName[0] == '*') 
		{
			/*
			* Valid wildcards:
			* - "*.domain.tld"
			* - "*.sub.domain.tld"
			* - etc.
			* Reject "*.tld".
			* No attempt to prevent the use of eg. "*.co.uk".
			*/

			/* Disallow "*" or "*." */
			if (certHostNameLen < 3)
				return false;
			
			if (certHostName[1] != '.')
				return false;
			/* Disallow "*.." */
			if (certHostName[2] == '.')
				return false;

			const char* nextDot = strchr(&certHostName[2], '.');
			/* Disallow "*.bar" */
			if (nextDot == nullptr)
				return false;
			/* Disallow "*.bar.." */
			if (strlen(nextDot) < 2 || nextDot[1] == '.')
				return false;

			auto found = hostname.find('.');
			/* No wildcard match against a name with no domain part. */
			if (found == hostname.npos)
				return false;

			std::string domain = hostname.substr(found);
			if (boost::iequals(
				domain,
				std::make_pair(certHostName + 1, certHostName + certHostNameLen - 1)))
				return true;
		}

		return false;
	};

	checkAltNames(func);
	checkCommonNames(func);
}

bool PeerID::Certificate::checkAltNames(
	const std::function<bool(char const*, std::size_t)> func) const
{
	GENERAL_NAMES* gens = static_cast<GENERAL_NAMES*>(
		X509_get_ext_d2i(x509_, NID_subject_alt_name, 0, 0));
	BOOST_SCOPE_EXIT(gens){
		GENERAL_NAMES_free(gens);
	}BOOST_SCOPE_EXIT_END;
	for (int i = 0; i < sk_GENERAL_NAME_num(gens); ++i)
	{
		GENERAL_NAME* gen = sk_GENERAL_NAME_value(gens, i);
		if (gen->type == GEN_DNS)
		{
			ASN1_IA5STRING* domain = gen->d.dNSName;
			if (domain->type == V_ASN1_IA5STRING && domain->data && domain->length)
			{
				const char* host = reinterpret_cast<const char*>(domain->data);
				std::size_t len = domain->length;
				if (func(host, len))
					return true;
			}
		}
	}
}

bool PeerID::Certificate::checkAltNames(boost::asio::ip::address& address) const
{
	GENERAL_NAMES* gens = static_cast<GENERAL_NAMES*>(
		X509_get_ext_d2i(x509_, NID_subject_alt_name, 0, 0));
	BOOST_SCOPE_EXIT(gens){
		GENERAL_NAMES_free(gens);
	}BOOST_SCOPE_EXIT_END;
	for (int i = 0; i < sk_GENERAL_NAME_num(gens); ++i)
	{
		GENERAL_NAME* gen = sk_GENERAL_NAME_value(gens, i);
		if (gen->type == GEN_IPADD)
		{			
			ASN1_OCTET_STRING* ip = gen->d.iPAddress;
			if (ip->type == V_ASN1_OCTET_STRING && ip->data)
			{
				return (
					(address.is_v4() && ip->length == 4 &&
						std::memcmp(address.to_v4().to_bytes().data(), ip->data, 4) == 0) ||
					(address.is_v6() && ip->length == 16 &&
						std::memcmp(address.to_v6().to_bytes().data(), ip->data, 16) == 0)
					);
			}
		}
	}
}

bool PeerID::Certificate::checkCommonNames(
	const std::function<bool(char const*, std::size_t)> func) const
{
	X509_NAME* name = X509_get_subject_name(x509_);
	int i = -1;
	ASN1_STRING* commonName = 0;
	while ((i = X509_NAME_get_index_by_NID(name, NID_commonName, i)) >= 0)
	{
		X509_NAME_ENTRY* nameEntry = X509_NAME_get_entry(name, i);
		commonName = X509_NAME_ENTRY_get_data(nameEntry);
		if (commonName && commonName->data && commonName->length)
		{
			const char* host = reinterpret_cast<const char*>(commonName->data);
			std::size_t len = commonName->length;
			if (func(host, len))
				return true;
		}
	}

	return false;
}

bool PeerID::Certificate::checkCommonNames(const boost::asio::ip::address& address) const
{
	std::string ipString = address.to_string();
	return checkCommonNames(
		[&ipString](char const * host, std::size_t len) -> bool
	{
		return boost::iequals(
			ipString,
			std::make_pair(host, host + len));
	});
}