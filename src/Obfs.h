#pragma once
#include "static.h"
#include "quickHash.h"
#include <openssl/chacha.h>
#include "chacha7.h"
#include "Stream.h"

class Obfs
{
public:
	enum { SECRET_BITS = 256 };
	enum { IV_BITS = 64 };
	using Ctx = ChaCha_ctx;
	Obfs(std::unique_ptr<Ctx>&& ctx)
		: ctx_(std::move(ctx))
	{}
	Obfs(Obfs&& o)
	{
		ctx_ = std::move(o.ctx_);
	}

	class Key
	{
	public:
		Key(const std::string& str);
		Key(const Key& o)
			: hash_(o.hash_)
		{}
		std::unique_ptr<Ctx> randBytes(char* randBytes) const;
		std::unique_ptr<Ctx> useRandBytes(char const* randBytes) const;
		std::pair<std::unique_ptr<Obfs::Ctx>, std::unique_ptr<Obfs::Ctx> >
			exchange(Stream* stream, boost::asio::yield_context yield) const;
		using Block = std::array < char, (SECRET_BITS + IV_BITS) / 8 > ;

	private:
		Block hash_;
	};

	void operator()(const void* in, void* out, std::size_t size)
	{
		::ChaCha7(ctx_.get(),
			reinterpret_cast<unsigned char*>(out),
			reinterpret_cast<const unsigned char*>(in),
			size
			);
	}
	void operator()(void* inOut, std::size_t size)
	{
		::ChaCha7(ctx_.get(),
			reinterpret_cast<unsigned char*>(inOut),
			reinterpret_cast<const unsigned char*>(inOut),
			size
			);
	}	

private:
	std::unique_ptr<Ctx> ctx_;
	unsigned char testInt_ = 0;
};