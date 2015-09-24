#include "RelaySession.h"
#include "Socks5Session.h"
#include "toBytes.h"

void Socks5Session::go()
{
	auto _ = shared_from_this();
	boost::asio::spawn(ios,
		[this, _](boost::asio::yield_context yield)
	{
		try
		{
			readMethod(yield);
			readRequest(yield);
		}
		catch (InvalidProtocol)
		{
		}
		catch (boost::system::system_error& /*e*/)
		{
		}
	});
}

void Socks5Session::readMethod(boost::asio::yield_context yield)
{
	readVersion(yield);

	auto nMethods = stream_->read(boost::asio::transfer_exactly(1), yield);
	unsigned char count = buffersCast<unsigned char>(nMethods);
	if (!count)
	{
		throw InvalidProtocol();
	}

	auto methods = stream_->read(boost::asio::transfer_exactly(count), yield);
	ConstStringFromBuffers constStringMethods(methods);
	auto found=std::find(constStringMethods.begin(), constStringMethods.end(), 0);
	if (found != constStringMethods.end())
	{
		std::array<unsigned char, 2> reply = { 0x05, 0x00 };
		stream_->write(boost::asio::buffer(reply), yield);
	}
	else
	{
		std::array<unsigned char, 2> reply = { 0x05, 0xFF };
		stream_->write(boost::asio::buffer(reply), yield);
	}
}

void Socks5Session::readRequest(boost::asio::yield_context yield)
{
	readVersion(yield);
	auto cmd = stream_->read(boost::asio::transfer_exactly(2), yield);

	switch (buffersCast<unsigned char>(cmd))
	{
	case 1:
		handleConnectRequest(yield);
		break;
	case 2:
		//bind
	case 3:
		//udp
	default:
		writeCommandNotSupoorted(yield);
	}
}

void Socks5Session::handleConnectRequest(boost::asio::yield_context yield)
{
	auto attp = stream_->read(boost::asio::transfer_exactly(1), yield);
	std::unique_ptr<Stream> stream2;
	boost::system::error_code ec;
	bool connectionAllowed;

	switch (buffersCast<unsigned char>(attp))
	{
	case 1:
	{
		auto data = stream_->read(boost::asio::transfer_exactly(4), yield);
		boost::asio::ip::address_v4::bytes_type ip4Bytes;
		buffersCastToSequence(data, ip4Bytes);
		boost::asio::ip::address_v4 ipv4(ip4Bytes);

		try
		{
			unsigned short port = readPort(yield);
			if (connectionAllowed=(*acl_)(stream_->remotePeerID(), ipv4, port))
			{
				stream2 = upStreamProxy_->connect(ipv4, port, yield);
			}
		}
		catch (boost::system::system_error& e)
		{
			ec=e.code();
		}
		break;
	}
	case 3:
	{
		auto n = stream_->read(boost::asio::transfer_exactly(1), yield);
		auto host = stream_->read(
			boost::asio::transfer_exactly(buffersCast<unsigned char>(n)), yield);
		std::string sHost = buffersCast<std::string>(host);

		try
		{
			unsigned short port = readPort(yield);
			if (connectionAllowed=(*acl_)(stream_->remotePeerID(), sHost, port))
			{
				stream2 = upStreamProxy_->connect(sHost, port, yield);
			}
		}
		catch (boost::system::system_error& e)
		{
			ec = e.code();
		}
		break;
	}
	case 4:
	{
		auto data = stream_->read(boost::asio::transfer_exactly(16), yield);
		boost::asio::ip::address_v6::bytes_type ip6Bytes;
		buffersCastToSequence(data, ip6Bytes);
		boost::asio::ip::address_v6 ipv6(ip6Bytes);

		try
		{
			unsigned short port = readPort(yield);
			if (connectionAllowed=(*acl_)(stream_->remotePeerID(), ipv6, port))

			{
				stream2 = upStreamProxy_->connect(ipv6, port, yield);
			}
		}
		catch (boost::system::system_error& e)
		{
			ec = e.code();
		}
		break;
	}
	default:
		throw InvalidProtocol();
	}

	if (connectionAllowed)
	{
		switch (ec.value())
		{
		case 0:
		{
			assert(stream2.get());
			char version[] = { 5, 0, 0 };
			char addressType;
			unsigned char address[16];

			std::vector<boost::asio::const_buffer> reply;
			reply.emplace_back(boost::asio::buffer(version, sizeof(reply)));
			const PeerID peerID = stream2->localPeerID();
			if (peerID.address.is_unspecified())
			{
				const char data[4] = { 0 };
				addressType = 1;
				reply.emplace_back(boost::asio::buffer(&addressType, 1));
				std::memset(address, 0, 4);
				reply.emplace_back(boost::asio::buffer(address, 4));
			}
			else if (peerID.address.is_v4())
			{
				addressType = 1;
				reply.emplace_back(boost::asio::buffer(&addressType, 1));
				std::memcpy(address, peerID.address.to_v4().to_bytes().data(), 4);
				reply.emplace_back(boost::asio::buffer(address, 4));
			}
			else /*if (peerID.address.is_v6())*/
			{
				addressType = 4;
				reply.emplace_back(boost::asio::buffer(&addressType, 1));
				std::memcpy(address, peerID.address.to_v6().to_bytes().data(), 16);
				reply.emplace_back(boost::asio::buffer(address, 16));
			}

			auto portBytes = toBytes(peerID.port);
			reply.emplace_back(boost::asio::buffer(portBytes.data(), portBytes.size()));

			stream_->write(reply, yield);

			relay(std::move(stream_), std::move(stream2));
			break;
		}
		case boost::asio::error::network_unreachable:
			writeError(3, yield);
			break;
		case boost::asio::error::host_unreachable:
			writeError(4, yield);
			break;
		case boost::asio::error::connection_refused:
			writeError(5, yield);
			break;
		default:
			writeError(1, yield);
			break;
		}
	}
	else	//connection not allowed
	{
		writeError(2, yield);
	}
}

unsigned short Socks5Session::readPort(boost::asio::yield_context yield)
{
	auto data = stream_->read(boost::asio::transfer_exactly(2), yield);
	return buffersCast<unsigned short>(data);
}

void Socks5Session::readVersion(boost::asio::yield_context yield)
{
	auto readVersion = stream_->read(boost::asio::transfer_exactly(1), yield);
	if (buffersCast<unsigned char>(readVersion) != 5)
	{
		throw InvalidProtocol();
	}
}

void Socks5Session::writeCommandNotSupoorted(boost::asio::yield_context yield)
{
	char reply[] = { 5, 7, 0, 1, 0, 0, 0, 0, 0, 0 };
	stream_->write(boost::asio::buffer(reply, sizeof(reply)), yield);
}

void Socks5Session::writeError(unsigned char errorCode, boost::asio::yield_context yield)
{
	stream_->writeByte(5, yield);
	stream_->writeByte(errorCode, yield);

	char reply[] = { 0, 1, 0, 0, 0, 0, 0, 0 };
	stream_->write(boost::asio::buffer(reply, sizeof(reply)), yield);
}
