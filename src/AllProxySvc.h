#pragma once
#include "static.h"
#include "Lobby.h"
#include "HttpsProxySvc.h"
#include "Socks5Svc.h"

class AllProxySvc
	: public Lobby
{
public:
	enum class InnerEntryEnum
	{
		https = 0,
		socks5,
		socks4,
		ssl,
		http,
		obfuscation,
		count
	};

protected:
	using InnerEntries = std::array < InnerEntry*, static_cast<std::size_t>(InnerEntryEnum::count) > ;
	InnerEntries innerEntries_;

	virtual void createSession(std::unique_ptr<Stream> stream);
	friend class AllProxySession;

public:
	AllProxySvc()
	{
		for (InnerEntry* &innerEntry : innerEntries_)
		{
			innerEntry = nullptr;
		}
	}

	virtual void killInnerEntry(InnerEntry* innerEntry);
	inline AllProxySvc& operator <<(std::shared_ptr<Socks5Svc> svc)
	{
		link(InnerEntryEnum::socks5, std::static_pointer_cast<Svc>(svc));
		return *this;
	}
	inline AllProxySvc& operator <<(std::shared_ptr<HttpsProxySvc> svc)
	{
		link(InnerEntryEnum::https, std::static_pointer_cast<Svc>(svc));
		return *this;
	}
	inline AllProxySvc& operator << (std::tuple<InnerEntryEnum, std::shared_ptr<Svc> > arg)
	{
		link(std::get<0>(arg), std::get<1>(arg));
		return *this;
	}
	void link(InnerEntryEnum innerEntryEnum, std::shared_ptr<Svc> svc);
};