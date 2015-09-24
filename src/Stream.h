#pragma once
#include "static.h"
#include "Buffers.h"
#include "PeerID.h"

class Stream
{
public:
	virtual ~Stream()
	{}

	using ReadHandler = std::function < void(const boost::system::error_code&, std::size_t) >;
	using WriteHandler = std::function < void(const boost::system::error_code&, std::size_t) >;

	virtual void readSome(const MutableBuffers& buffers, const ReadHandler& handler) = 0;
	virtual void writeSome(const ConstBuffers& buffers, const WriteHandler& handler) = 0;
	virtual boost::system::error_code eof(boost::asio::yield_context yield)=0;
	//virtual void cancel() = 0;
	virtual boost::system::error_code close(boost::asio::yield_context yield) = 0;
	virtual void close()
	{
		boost::asio::spawn(ios,
			[this](boost::asio::yield_context yield)
		{
			close(yield);
		});
	}
	virtual PeerID localPeerID() const = 0;
	virtual PeerID remotePeerID() const = 0;

	inline MutableOwnedBuffers readLine(boost::asio::yield_context yield)
	{
		return readUntil(
			[](const ConstBuffers& bufs) -> std::size_t
		{
			ConstStringFromBuffers sbufs(bufs);
			return (sbufs.empty() || sbufs.back() != '\n')?1:0;
		}
			, yield);
	}

	//inline MutableOwnedBuffers readSome(ReadHandler handler)
	//{
	//	MutableOwnedBuffers bufs(bufferSize());
	//	readSome(bufs, handler);
	//	return bufs;
	//}

	inline MutableOwnedBuffers readSome(boost::asio::yield_context yield)
	{
		return read(boost::asio::transfer_at_least(1), yield);
	}

	template <typename CompletionCondition>
	inline MutableOwnedBuffers read(CompletionCondition completionCondition, boost::asio::yield_context yield)
	{
		MutableOwnedBuffers bufs;
		if (!readBufs_.empty())
		{
			bufs = std::move(readBufs_);
			if (completionCondition(0, bufs.size()) == 0)
			{
				return bufs;
			}
		}
		else
		{
			bufs.increase(bufferSize());
		}

		std::size_t n = 
			boost::asio::async_read(wrapper_, 
				boost::asio::mutable_buffers_1(bufs.back() + bufs.lastBufferSize()), 
				completionCondition, yield);
		bufs.lastTransferred(n);
		return bufs;
	}

	template <typename T>
	inline T readValue(boost::asio::yield_context yield)
	{
		MutableOwnedBuffers buffers = read(
			boost::asio::transfer_exactly(sizeof(T)), yield);
		ConstStringFromBuffers b2(buffers);
		T ret = 0;
		auto it = b2.begin();
		for (std::size_t i = 0; i < sizeof(T); ++i, ++it)
		{
			ret <<= 8;
			ret |= static_cast<unsigned char>(*it);
		}
		return ret;
	}
	//peak must be called before any read operation
	//the first read must consume more bytes then peak
	inline MutableBuffers peak(std::size_t size, boost::asio::yield_context yield)
	{
		std::size_t n =
			boost::asio::async_read(wrapper_, 
				boost::asio::mutable_buffers_1(readBufs_.back() + readBufs_.lastBufferSize()),
				boost::asio::transfer_exactly(size), yield);

		readBufs_.lastTransferred(n);
		return static_cast<std::vector < boost::asio::mutable_buffer >>(readBufs_);
	}

	template <typename CompletionCondition>
	inline MutableOwnedBuffers readUntil(CompletionCondition completionCondition, boost::asio::yield_context yield)
	{
		MutableOwnedBuffers bufs;
		if (!readBufs_.empty())
		{
			bufs = std::move(readBufs_);
			std::size_t more;
			if ((more=completionCondition(ConstBuffers(bufs))) == 0)
			{
				return bufs;
			}
		}
		else
		{
			bufs.increase(bufferSize());
		}


		std::size_t more;
		while ((more = completionCondition(ConstBuffers(bufs))) > 0)
		{
			boost::asio::mutable_buffer buf = bufs.back()+bufs.lastBufferSize();
			
			std::size_t n = boost::asio::async_read(wrapper_, 
				MutableBuffers(buf), boost::asio::transfer_exactly(more), yield);
			bufs.lastTransferred(n);

			if (bufs.lastBufferSize() == boost::asio::buffer_size(bufs.back()))
			{
				bufs.increase(bufferSize());
			}
		}
		return bufs;
	}

	std::size_t writeSome(const ConstBuffers& buffers, boost::asio::yield_context yield)
	{
		return write(buffers, boost::asio::transfer_at_least(1), yield);
	}

	inline std::size_t writeByte(char c, boost::asio::yield_context yield)
	{
		return write(boost::asio::buffer(&c, 1), boost::asio::transfer_all(), yield);
	}

	template <typename T>
	inline std::size_t writeValue(T t, boost::asio::yield_context yield)
	{
		std::array<char, sizeof(T)> data;
		for (std::size_t i = 0; i < data.size(); ++i)
		{
			data[i] = t >> ((sizeof(T) - i - 1) * 8);
		}

		return write(boost::asio::buffer(data, data.size()), boost::asio::transfer_all(), yield);
	}

	inline std::size_t write(const ConstBuffers & buffers, boost::asio::yield_context yield)
	{
		return write(buffers, boost::asio::transfer_all(), yield);
	}

	template <typename CompletionCondition>
	inline std::size_t write(const ConstBuffers & buffers, CompletionCondition completionCondition, boost::asio::yield_context yield)
	{
		return boost::asio::async_write(wrapper_, buffers, completionCondition, yield);
	}

	inline void write(const ConstBuffers & buffers, const WriteHandler& handler)
	{
		write(buffers, boost::asio::transfer_all(), handler);
	}

	template <typename CompletionCondition>
	inline void write(const ConstBuffers & buffers, CompletionCondition completionCondition, const WriteHandler& handler)
	{
		boost::asio::async_write(wrapper_, buffers, completionCondition, handler);
	}

	inline void bufferSize(std::size_t bufferSize)
	{
		bufferSize_ = bufferSize;
	}

	inline std::size_t bufferSize() const
	{
		return bufferSize_;
	}

	inline std::size_t bufferSizeAtleast(std::size_t bufferSize)
	{
		return bufferSize_ = std::max(bufferSize, bufferSize_);
	}
private:
	struct Wrapper
	{
		Wrapper(Stream* stream)
			: stream(stream)
		{}

		inline boost::asio::io_service& get_io_service() const
		{
			return ios;
		}


		template <typename Buffers, typename H>
		void async_read_some(const Buffers& buffers, H h)
		{
			return stream->readSome(buffers, h);
		}
		template <typename H>
		void async_write_some(const ConstBuffers& buffers, H h)
		{
			return stream->writeSome(buffers, h);
		}

		Stream * stream;
	} wrapper_ = {this};
	std::size_t bufferSize_=128;

	MutableOwnedBuffers readBufs_{ bufferSize_ };
};