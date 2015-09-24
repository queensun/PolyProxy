#include "Obfs.h"
#include "quickHash.h"

namespace
{
	std::default_random_engine gen;
	int init()
	{
		gen.seed(std::time(0));
		return 0;
	}
	int dummy = init();
}

Obfs::Key::Key(const std::string& str)
{
	quickHash320(str.data(), str.length(), hash_.data());
}

std::unique_ptr<Obfs::Ctx> Obfs::Key::randBytes(char* randBytes) const
{
	static std::uniform_int_distribution<int> dist(-128, 127);
	std::generate(randBytes, randBytes + (SECRET_BITS + IV_BITS) / 8, std::bind(dist, std::ref(gen)));
	randBytes[0] |= 0x80;

	return useRandBytes(randBytes);
}

std::unique_ptr<Obfs::Ctx> Obfs::Key::useRandBytes(char const* randBytes) const
{
	std::unique_ptr<Obfs::Ctx> ctx(new Obfs::Ctx);
	::ChaCha_set_key(ctx.get(), reinterpret_cast<const unsigned char*>(randBytes), 32);
	::ChaCha_set_iv(ctx.get(), reinterpret_cast<const unsigned char*>(randBytes + 32), 0);
	return std::move(ctx);
}

std::pair<std::unique_ptr<Obfs::Ctx>, std::unique_ptr<Obfs::Ctx> >
Obfs::Key::exchange(Stream* stream, boost::asio::yield_context yield) const
{
	static const char obfs[4] = { 'o', 'b', 'f', 's' };
	std::pair<std::unique_ptr<Obfs::Ctx>, std::unique_ptr<Obfs::Ctx> > ret;
	{
		{
			//send bytes for exchange
			Obfs::Key::Block rand;
			ret.first = randBytes(rand.data());
			stream->write(boost::asio::buffer(rand.data(), rand.size()), yield);
		}

		{
			//send random bytes
			unsigned char bytes = 0;
			::ChaCha7(ret.first.get(), &bytes, &bytes, 1);
			bytes &= 0x7F;
			if (bytes > 0)
			{
				char dummy[128];
				static std::uniform_int_distribution<int> dist(-128, 127);
				std::generate(dummy, dummy + bytes, std::bind(dist, std::ref(gen)));
				stream->write(boost::asio::buffer(dummy, bytes), yield);
			}
		}

		{
			//send verification bytes
			unsigned char enc[4];
			::ChaCha7(ret.first.get(), enc, reinterpret_cast<const unsigned char*>(obfs), 4);
			stream->write(boost::asio::buffer(enc, 4), yield);
		}

		//skip to 1024 bytes
		ret.first->unused = 0;
		ret.first->input[12] = 1024;
	}
	{
		{
			//receive bytes for exchange
			const std::size_t bufSize = (Obfs::SECRET_BITS + Obfs::IV_BITS) / 8;
			stream->bufferSizeAtleast(bufSize);
			auto bufs = stream->read(boost::asio::transfer_exactly(bufSize), yield);
			const char* rand = boost::asio::buffer_cast<char const*>(bufs.front());
			ret.second = useRandBytes(rand);
		}

		{
			//receive random bytes
			unsigned char bytes = 0;
			::ChaCha7(ret.second.get(), &bytes, &bytes, 1);
			bytes &= 0x7F;
			stream->read(boost::asio::transfer_exactly(bytes), yield);
		}

		{
			//receive verification bytes
			stream->bufferSizeAtleast(4);
			auto bufs=stream->read(boost::asio::transfer_exactly(4), yield);
			unsigned char dec[4];
			::ChaCha7(ret.second.get(), dec, boost::asio::buffer_cast<unsigned char const*>(bufs.front()), 4);
			if (memcmp(dec, obfs, 4) != 0)
			{
				NetworkErrorCat::throwError(NetworkError::protocolError);
			}
		}

		//skip to 1024 bytes
		ret.second->unused = 0;
		ret.second->input[12] = 1024;
	}
	return ret;
}