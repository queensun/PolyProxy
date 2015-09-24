#include "SslStream.h"
#include <openssl/bio.h>
#include <boost/scope_exit.hpp>

SslStream::SslStream(std::unique_ptr<Stream> &&nextLayer, SslContext& context)
	: socket_(*nextLayer.get(), context)
	, localPeerID_(nextLayer->localPeerID())
{
	nextLayer_ = std::move(nextLayer);
	X509* cert = ::SSL_get_certificate(socket_.native_handle());
	localPeerID_.certificate = PeerID::Certificate(true, cert);
}

void SslStream::clientHandShake(const std::string& sni, boost::asio::yield_context yield)
{
	if (!sni.empty())
	{
		::SSL_set_tlsext_host_name(socket_.native_handle(), sni.c_str());
	}
	//socket_.set_verify_callback(
	//	boost::asio::ssl::rfc2818_verification::rfc2818_verification(name));
	clientHandShake(yield);
}

void SslStream::clientHandShake(boost::asio::yield_context yield)
{
	socket_.async_handshake(HandShakeType::client, yield);
	bool valid = (::SSL_get_verify_result(socket_.native_handle()) == X509_V_OK);

	X509* cert=::SSL_get_peer_certificate(socket_.native_handle());
	BOOST_SCOPE_EXIT(cert){
		X509_free(cert);
	}BOOST_SCOPE_EXIT_END;

	remotePeerID_ = nextLayer_->remotePeerID();
	remotePeerID_.certificate = PeerID::Certificate(valid, cert);
}

void SslStream::serverHandShake(boost::asio::yield_context yield)
{
	socket_.async_handshake(HandShakeType::server, yield);
	bool valid = (::SSL_get_verify_result(socket_.native_handle()) == X509_V_OK);

	X509* cert = ::SSL_get_peer_certificate(socket_.native_handle());
	BOOST_SCOPE_EXIT(cert){
		X509_free(cert);
	}BOOST_SCOPE_EXIT_END;

	remotePeerID_ = nextLayer_->remotePeerID();
	remotePeerID_.certificate = PeerID::Certificate(valid, cert);
}

void SslStream::readSome(const MutableBuffers& buffers, const ReadHandler& handler)
{
	socket_.async_read_some(buffers, handler);
}

void SslStream::writeSome(const ConstBuffers& buffers, const WriteHandler& handler)
{
	socket_.async_write_some(buffers, handler);
}

boost::system::error_code SslStream::eof(boost::asio::yield_context yield)
{
	if (!closed_ && !finSent_)
	{
		try
		{
			socket_.async_shutdown(yield);
		}
		catch (boost::system::system_error& e)
		{
			return e.code();
		}
	}
	return boost::system::error_code();
}
//
//void SslStream::cancel()
//{
//	nextLayer_->cancel();
//}

boost::system::error_code SslStream::close(boost::asio::yield_context yield)
{
	closed_ = true;
	return nextLayer_->close(yield);
}

void SslStream::close()
{
	closed_ = true;
	return nextLayer_->close();
}

PeerID SslStream::localPeerID() const
{
	return localPeerID_;
}

PeerID SslStream::remotePeerID() const
{
	return remotePeerID_;
}