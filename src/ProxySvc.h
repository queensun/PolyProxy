#pragma once
#include "static.h"
#include "Svc.h"
#include "UpStreamProxy.h"
#include <boost/logic/tribool.hpp>

class ProxySvc
	: public Svc
{
public:
	class Acl
	{
	public:
		inline bool operator ()(
			const PeerID& peerID, const std::string& host, unsigned port)
		{
			boost::tribool tmp = allowed(peerID, host, port);
			if (!boost::indeterminate(tmp))
			{
				return tmp;
			}
			else
			{
				return allowed(peerID);
			}
		}

		inline bool operator ()(
			const PeerID& peerID, const boost::asio::ip::address& address, unsigned port)
		{
			boost::tribool tmp = allowed(peerID, address, port);
			if (!boost::indeterminate(tmp))
			{
				return tmp;
			}
			else
			{
				return allowed(peerID);
			}
		}

		inline bool operator ()(const PeerID& peerID)
		{
			return allowed(peerID);
		}

		virtual std::shared_ptr<Acl> create() const
		{
			return std::shared_ptr<Acl>(new Acl());
		}

	protected:
		virtual boost::tribool allowed(
			const PeerID& peerID, const std::string& host, unsigned port)
		{
			return true;
		}
		virtual boost::tribool allowed(
			const PeerID& peerID, const boost::asio::ip::address& address, unsigned port)
		{
			return true;
		}
		virtual bool allowed(const PeerID& peerID)
		{
			return true;
		}
	};


	inline void upStreamProxy(std::shared_ptr<UpStreamProxy> o)
	{
		if (status() != Status::created)
		{
			throw InvalidAssignmentExcept();
		}

		upStreamProxy_ = o;
	}

	inline std::shared_ptr<UpStreamProxy> upStreamProxy() const
	{
		return upStreamProxy_;
	}

	inline void acl(std::shared_ptr<Acl> o)
	{
		if (status() != Status::created)
		{
			throw InvalidAssignmentExcept();
		}

		acl_ = o;
	}

	inline std::shared_ptr<Acl> acl() const
	{
		return acl_;
	}

private:
	std::shared_ptr<UpStreamProxy> upStreamProxy_ = defaultUpStreamProxy;
	std::shared_ptr<Acl> acl_ = defaultAcl->create();

	static std::unique_ptr<Acl> defaultAcl;
};