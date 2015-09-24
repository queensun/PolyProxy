#pragma once
#include "Stream.h"
#include "SslContext.h"

class SslStream
	: public Stream
{
protected:
	struct Wrapper
	{
		Wrapper(Stream* stream)
			: stream(stream)
		{
		}
		Wrapper(Stream& stream)
			: stream(&stream)
		{
		}

		struct lowest_layer_type
		{
			inline boost::asio::io_service & get_io_service()
			{
				return ios;
			}
		};

		inline const lowest_layer_type & lowest_layer() const
		{
			static lowest_layer_type ret;
			return ret;
		}

		inline lowest_layer_type & lowest_layer()
		{
			static lowest_layer_type ret;
			return ret;
		}

		inline boost::asio::io_service & get_io_service()
		{
			return ios;
		}

		template<
			typename MutableBufferSequence,
			typename ReadHandler>
			void async_read_some(
			const MutableBufferSequence & buffers,
			ReadHandler handler)
		{
			stream->readSome(buffers, handler);
		}

		template<
			typename ConstBufferSequence,
			typename WriteHandler>
			void async_write_some(
			const ConstBufferSequence & buffers,
			WriteHandler handler)
		{
			stream->writeSome(buffers, handler);
		}

		Stream* stream;
	};
	
public:
	SslStream(std::unique_ptr<Stream> &&nextLayer, SslContext& context);

	virtual void readSome(const MutableBuffers& buffers, const ReadHandler& handler);
	virtual void writeSome(const ConstBuffers& buffers, const WriteHandler& handler);
	virtual boost::system::error_code eof(boost::asio::yield_context yield);
	//virtual void cancel();
	virtual boost::system::error_code close(boost::asio::yield_context yield);
	virtual void close();
	virtual PeerID localPeerID() const;
	virtual PeerID remotePeerID() const;

	void clientHandShake(const std::string& sni, boost::asio::yield_context yield);
	void clientHandShake(boost::asio::yield_context yield);
	void serverHandShake(boost::asio::yield_context yield);

	SSL* nativeHandle()
	{
		return socket_.native_handle();
	}

	using Socket = boost::asio::ssl::stream < Wrapper > ;

protected:
	using HandShakeType = Socket::handshake_type;
	Socket socket_;
	std::unique_ptr<Stream> nextLayer_;

	PeerID localPeerID_, remotePeerID_;
	bool closed_ = false;
	bool finSent_ = false;
};