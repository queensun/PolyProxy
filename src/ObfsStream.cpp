#include "ObfsStream.h"

template <typename Base>
void ObfsStream<Base>::readSome(const MutableBuffers& buffers, const ReadHandler& handler)
{
	Base::readSome(buffers, 
		[this, buffers, handler](const boost::system::error_code& ec, std::size_t size)
	{
		std::size_t remaining = size;
		for (boost::asio::mutable_buffer b : buffers)
		{
			unsigned char* bData = boost::asio::buffer_cast<unsigned char*>(b);
			std::size_t bSize = boost::asio::buffer_size(b);
			std::size_t bytesToRead = std::min(bSize, remaining);
			obfsRead_(bData, bytesToRead);
			remaining -= bytesToRead;

			if (!remaining)
			{
				break;
			}
		}

		handler(ec, size);
	});
}

template <typename Base>
void ObfsStream<Base>::writeSome(const ConstBuffers& buffers, const WriteHandler& handler)
{
	ConstStringFromBuffers s(buffers);
	std::size_t bufSize = s.size();	

	if (bufSize > 0)
	{
		int secNeeded = static_cast<int>(bufSize) - secBytesQueue_.size();
		if (secNeeded > 0)
		{
			secNeeded = secNeeded / 128 * 128 + 128;
			std::vector<char> v(secNeeded);
			std::fill(v.begin(), v.end(), 0);
			obfsWrite_(v.data(), v.size());
			secBytesQueue_.add(std::move(v));
		}		

		writeBuf_.resize(bufSize);
		std::transform(s.cbegin(), s.cend(), secBytesQueue_.cbegin(), writeBuf_.begin(),
			[](char c1, char c2)
		{
			return c1 ^ c2;
		});

		Base::writeSome(
			boost::asio::const_buffers_1(writeBuf_.data(), writeBuf_.size()),
			[this, handler](const boost::system::error_code& ec, std::size_t size)
		{
			secBytesQueue_.consume(size);
			handler(ec, size);
		});
	}
	else
	{
		Base::writeSome(buffers, handler);
	}
}

template class ObfsStream<TcpStream>;
template class ObfsStream<LANTcpStream>;