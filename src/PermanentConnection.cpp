#include "PermanentConnection.h"
#include "PermanentConnectionStream.h"
#include "PermanentConnectionEntry.h"
#include "toBytes.h"
#include "SslStream.h"

PermanentConnection::PermanentConnection(
	std::unique_ptr<Stream>&& stream,
	bool active)
	: stream_(std::move(stream))
	, active_(active)
	, localPeerID_(stream_->localPeerID())
	, remotePeerID_(stream_->remotePeerID())
	, sslContext_(new SslContext)
{
	//::SSL_CTX_set_cipher_list(sslContext_->native_handle(), "NULL-SHA");
}

void PermanentConnection::sendKeepAlive(boost::asio::yield_context yield)
{
	char data[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	write(boost::asio::buffer(data, sizeof(data)), yield);
}

std::unique_ptr<Stream> PermanentConnection::open(
	const std::string& virtualService, boost::asio::yield_context yield)
{
	if (streamInfos_.size() > maxStreams_)
	{
		NetworkErrorCat::throwError(NetworkError::insufficientResources);
	}
	if (virtualService.size() > MAX_TRANSFER_SIZE)
	{
		throw boost::system::system_error(
			boost::system::error_code(boost::asio::error::access_denied)
			);
	}

	std::uint32_t streamID = newStreamID();
	StreamInfo &streamInfo = streamInfos_[streamID];
	write(Command::open, streamID,
		boost::asio::buffer(virtualService), yield
		);


	StreamInfo::Message message = read(streamInfo, yield);

	switch (message.command)
	{
	case Command::opened:
	{
		PeerID remotePeerID = remotePeerID_;
		remotePeerID.virtualService = virtualService;
		std::unique_ptr<Stream> ret(new PermanentConnectionStream(
			streamID,
			streamInfo,
			shared_from_this(),
			localPeerID_,
			remotePeerID
			));
		return ret;
	}
	case Command::refused:
		throw boost::system::system_error(
			boost::system::error_code(boost::asio::error::access_denied)
			);
	case Command::reset:
		throw boost::system::system_error(
			boost::system::error_code(boost::asio::error::connection_reset)
			);
		break;
	default:
		NetworkErrorCat::throwError(NetworkError::generalError);
	}
}

bool PermanentConnection::sendCertificate(boost::asio::yield_context yield)
{
	if (streamInfos_.size() > maxStreams_)
	{
		NetworkErrorCat::throwError(NetworkError::insufficientResources);
	}
	std::uint32_t streamID = newStreamID();
	StreamInfo &streamInfo = streamInfos_[streamID];
	write(Command::sendCertificate, streamID,
		boost::asio::null_buffers(), yield
		);

	StreamInfo::Message message = read(streamInfo, yield);

	switch (message.command)
	{
	case Command::opened:
	{
		std::unique_ptr<Stream> stream(new PermanentConnectionStream(
			streamID,
			streamInfo,
			shared_from_this(),
			localPeerID_,
			remotePeerID_
			));

		SslStream sslStream(std::move(stream), *sslContext_);
		sslStream.serverHandShake(yield);
		remotePeerID_.certificate = sslStream.remotePeerID().certificate;
		return true;
	}
	default:
		return false;
	}

}

void PermanentConnection::requestCertificate(boost::asio::yield_context yield)
{
	if (streamInfos_.size() > maxStreams_)
	{
		NetworkErrorCat::throwError(NetworkError::insufficientResources);
	}
	std::uint32_t streamID = newStreamID();
	StreamInfo &streamInfo = streamInfos_[streamID];
	write(Command::requestCertificate, streamID,
		boost::asio::null_buffers(), yield
		);

	StreamInfo::Message message = read(streamInfo, yield);

	switch (message.command)
	{
	case Command::opened:
	{
		std::unique_ptr<Stream> stream(new PermanentConnectionStream(
			streamID,
			streamInfo,
			shared_from_this(),
			localPeerID_,
			remotePeerID_
			));

		SslStream sslStream(std::move(stream), *sslContext_);
		sslStream.clientHandShake(yield);
		remotePeerID_.certificate = sslStream.remotePeerID().certificate;
		break;
	}
	default:
		break;
	}

}


std::size_t PermanentConnection::write(
	Command command,
	std::uint32_t streamID,
	const ConstBuffers & buffers,
	boost::asio::yield_context yield)
{
	std::shared_ptr < PermanentConnection::WritingBuffer > wb;
	std::vector < boost::asio::const_buffer > data;
	std::tie(wb, data) = fillInWriteBuffer(command, streamID, buffers);
	return write(data, yield) - HEADER_SIZE;
}

void PermanentConnection::write(
	Command command,
	std::uint32_t streamID,
	const ConstBuffers & buffers,
	const Stream::WriteHandler& handler)
{
	std::shared_ptr < PermanentConnection::WritingBuffer > wb;
	std::vector < boost::asio::const_buffer > data;
	std::tie(wb, data) = fillInWriteBuffer(command, streamID, buffers);

	struct Helper
	{
		Helper(
			std::shared_ptr < PermanentConnection::WritingBuffer > wb,
			const Stream::WriteHandler& handler)
			: wb(wb)
			, handler(handler)
		{}
		Helper(const Helper& o)
			: wb(o.wb)
			, handler(o.handler)
		{}

		std::shared_ptr < PermanentConnection::WritingBuffer > wb;
		Stream::WriteHandler handler;

		void operator ()(const boost::system::error_code& ec, std::size_t bytes)
		{
			assert(bytes > HEADER_SIZE);
			handler(ec, bytes - HEADER_SIZE);
		}
	};
	write(data, Helper(wb, handler));
}

std::tuple<std::shared_ptr<PermanentConnection::WritingBuffer>, std::vector<boost::asio::const_buffer> >
	PermanentConnection::fillInWriteBuffer(
	Command command,
	std::uint32_t streamID,
	const ConstBuffers & buffers)
{
	std::tuple < 
		std::shared_ptr<PermanentConnection::WritingBuffer>, 
		std::vector<boost::asio::const_buffer> 
	> ret;
	std::shared_ptr<WritingBuffer>& wb = std::get<0>(ret);
	std::vector<boost::asio::const_buffer>& data = std::get<1>(ret);

	wb.reset(new WritingBuffer);
	std::size_t size = 0;

	wb->command = toBytes<2>(static_cast<boost::uint16_t>(command));
	wb->streamID = toBytes<4>(streamID);

	data.assign({
		boost::asio::buffer(wb->command, 2),
		boost::asio::buffer(wb->streamID, 4),
		boost::asio::buffer(wb->len, 2)
	});

	for (const boost::asio::const_buffer &b : buffers)
	{
		std::size_t bufSize = boost::asio::buffer_size(b);
		if (bufSize == 0)
			continue;

		if (size + bufSize < MAX_TRANSFER_SIZE)
		{
			data.push_back(b);
			size += bufSize;
		}
		else
		{
			data.push_back(
				boost::asio::buffer(
				boost::asio::buffer_cast<const void*>(b),
				MAX_TRANSFER_SIZE - size
				));
			size = MAX_TRANSFER_SIZE;
			break;
		}
	}

	wb->len = toBytes<2>(size);
	std::cout << "about to write " << static_cast<int>(command) << ':' << streamID << ':' << size << std::endl;
	return ret;
}

std::size_t PermanentConnection::write(
	const ConstBuffers & buffers,
	boost::asio::yield_context yield)
{
	auto _ = shared_from_this();
	BOOST_SCOPE_EXIT(&writing_, &writeTimer_, &keepAliveTimer_){
		writing_ = false;
		writeTimer_.cancel();
		keepAliveTimer_.cancel();
	} BOOST_SCOPE_EXIT_END;

	while (writing_)
	{
		boost::system::error_code ec;
		writeTimer_.async_wait(yield[ec]);
	}
	writing_ = true;

	try
	{
		std::size_t ret = stream_->write(buffers, yield);
		return ret;
	}
	catch (boost::system::system_error&)
	{
		end(yield);
		throw;
	}
}

void PermanentConnection::write(
	const ConstBuffers &buffers,
	const Stream::WriteHandler& handler)
{
	auto _ = shared_from_this();
	auto writingHelper =
		[this, buffers, handler, _]()
	{
		writing_ = true;
		stream_->write(buffers, 
			[this, handler, _](const boost::system::error_code& ec, std::size_t size)
		{
			BOOST_SCOPE_EXIT(&writing_, &writeTimer_, &keepAliveTimer_){
				writing_ = false;
				writeTimer_.cancel();
				keepAliveTimer_.cancel();
			} BOOST_SCOPE_EXIT_END;

			handler(ec, size);
			if (ec)
			{
				boost::asio::spawn(ios,
					[this, _](boost::asio::yield_context yield)
				{
					end(yield);
				});
			}
		});
	};
	using WritingHelper = decltype(writingHelper);
	if (writing_)
	{
		struct WaitHelper
		{
			WaitHelper(WritingHelper &&writingHelper, PermanentConnection* that)
				: writingHelper(std::move(writingHelper)), that(that)
			{}
			WaitHelper(WaitHelper&& o)
				: writingHelper(std::move(o.writingHelper))
				, that(o.that)
			{}
			WaitHelper(const WaitHelper& o)
				: writingHelper(o.writingHelper)
				, that(o.that)
			{}			

			WritingHelper writingHelper;
			PermanentConnection* that;

			void operator() (const boost::system::error_code&)
			{
				if (!that->writing_)
				{
					writingHelper();
				}
				else
				{
					that->writeTimer_.async_wait(std::move(*this));
				}
			}
		} waitHelper{ std::move(writingHelper), this };
		writeTimer_.async_wait(std::move(waitHelper));
	}
	else
	{		
		writingHelper();
	}
}

void PermanentConnection::createReadCoroutine()
{
	auto _ = shared_from_this();
	boost::asio::spawn(ios,
		[this, _](boost::asio::yield_context yield)
	{
		try
		{
			while (1)
			{
				Command command = static_cast<Command>(stream_->readValue<boost::uint16_t>(yield));
				boost::uint32_t streamID = stream_->readValue<boost::uint32_t>(yield);
				boost::uint16_t len = stream_->readValue<boost::uint16_t>(yield);
				auto data = stream_->read(boost::asio::transfer_exactly(len), yield).release();

				std::cout << "reading: " << static_cast<int>(command) << ':' << streamID << ':' << len << std::endl;
				if (streamID)
				{
					if (command == Command::open || command==Command::sendCertificate || command==Command::requestCertificate)
					{
						if (streamInfos_.find(streamID) == streamInfos_.end())
						{
							if (command == Command::open)
							{
								std::string name;
								for (auto& buf : data)
								{
									name.insert(name.end(), buf.begin(), buf.end());
								}

								boost::asio::spawn(ios,
									[this, name, streamID, _](boost::asio::yield_context yield)
								{
									PeerID localPeerID = localPeerID_;
									localPeerID.virtualService = name;
									auto found = virtualServices_.find(name);
									try
									{
										if (found != virtualServices_.end())
										{
											write(Command::opened, streamID, boost::asio::null_buffers(), yield);
											PermanentConnectionEntry* entry = found->second;
											entry->passStream(
												std::unique_ptr<PermanentConnectionStream>(
													new PermanentConnectionStream(
														streamID,
														streamInfos_[streamID],
														_,
														localPeerID,
														remotePeerID_
													)
												), yield);
										}
										else /*if (found == virtualServices_.end())*/
										{
											write(Command::refused, streamID, boost::asio::null_buffers(), yield);
										}
									}
									catch (boost::system::system_error&)
									{
										end(yield);
									}
								});
							}
							else if (command == Command::sendCertificate)
							{
								if (remotePeerID_.certificate)
								{
									write(Command::refused, streamID, boost::asio::null_buffers(),
										[](const boost::system::error_code&, std::size_t){});
								}
								else
								{
									boost::asio::spawn(ios,
										[this, streamID, _](boost::asio::yield_context yield)
									{
										try
										{
											write(Command::opened, streamID, boost::asio::null_buffers(), yield);
											std::unique_ptr<Stream> stream (new PermanentConnectionStream(
												streamID,
												streamInfos_[streamID],
												_,
												localPeerID_,
												remotePeerID_
												));
											SslStream sslStream(std::move(stream), *sslContext_);
											sslStream.clientHandShake(yield);
											remotePeerID_.certificate = sslStream.remotePeerID().certificate;
										}
										catch (boost::system::system_error&)
										{
										}
									});
								}
							}
							else
							{
								assert(command == Command::requestCertificate);
								boost::asio::spawn(ios,
									[this, streamID, _](boost::asio::yield_context yield)
								{
									try
									{
										write(Command::opened, streamID, boost::asio::null_buffers(), yield);
										std::unique_ptr<Stream> stream(new PermanentConnectionStream(
											streamID,
											streamInfos_[streamID],
											_,
											localPeerID_,
											remotePeerID_
											));
										SslStream sslStream(std::move(stream), *sslContext_);
										sslStream.serverHandShake(yield);
										remotePeerID_.certificate = sslStream.remotePeerID().certificate;
									}
									catch (boost::system::system_error&)
									{										
									}
								});
							}
						}
						else
						{
							assert((streamInfos_.find(streamID) != streamInfos_.end()));
							write(Command::reset, streamID, boost::asio::null_buffers(),
								[](const boost::system::error_code&, std::size_t){});
						}
					}
					else
					{
						assert(command != Command::open && command != Command::sendCertificate && command != Command::requestCertificate);
						auto found = streamInfos_.find(streamID);
						if (found != streamInfos_.end())
						{
							StreamInfo &streamInfo = found->second;
							auto &messages = streamInfo.messages;
							for (auto &b : data)
							{
								messages.push(StreamInfo::Message{
									command,
									std::move(b)
								});
							}
						}
						else if (command!=Command::reset)
						{
							write(Command::reset, streamID, boost::asio::null_buffers(),
								[](const boost::system::error_code&, std::size_t){});
						}
					}
				} /*if (streamID*/
			} /*while(1)*/
		}
		catch (boost::system::system_error&)
		{
		}

		end(yield);
	});
}

void PermanentConnection::createKeepAliveCoroutine()
{
	auto _ = shared_from_this();
	boost::asio::spawn(ios,
		[this, _](boost::asio::yield_context yield)
	{
		try
		{
			while (keepAlive_)
			{
				boost::system::error_code ec;
				keepAliveTimer_.expires_from_now(std::chrono::seconds(keepAlive_));
				keepAliveTimer_.async_wait(yield[ec]);
				if (!ec)
				{
					sendKeepAlive(yield);
				}
			}
		}
		catch (boost::system::system_error&)
		{
		}
	});
}


void PermanentConnection::end(boost::asio::yield_context yield)
{
	zombie_ = true;
	keepAlive(0);
	for (auto &v : streamInfos_)
	{
		StreamInfo &streamInfo = v.second;
		streamInfo.messages.push(StreamInfo::Message{ Command::reset });
	}
	while (!virtualServices_.empty())
	{
		PermanentConnectionEntry* entry =
			virtualServices_.begin()->second;
		entry->stop(yield);
	}
	if (reconnectFunc_)
	{
		reconnectFunc_();
		reconnectFunc_ = nullptr;
	}
}

PermanentConnection::StreamInfo::Message PermanentConnection::read(
	StreamInfo& streamInfo, boost::asio::yield_context yield)
{
	try
	{
		StreamInfo::Message ret = streamInfo.messages.pop(yield);
		return ret;
	}
	catch (InterCoroutineVarEndedExcept&)
	{
		throw boost::system::error_code(boost::asio::error::connection_reset);
	}
}

void PermanentConnection::read(
	std::int32_t streamID,
	StreamInfo& streamInfo,
	const MutableBuffers& buffers,
	const Stream::ReadHandler& readHandler)
{
	auto _ = shared_from_this();
	auto readHelper = 
		[this, streamID, buffers, readHandler, _](InterCoroutineVar<StreamInfo::Message>::List &messages)
	{
		std::size_t sizeTotal = 0;
		boost::system::error_code ec;

		BOOST_SCOPE_EXIT(&sizeTotal, &ec, &readHandler){
			readHandler(ec, sizeTotal);
		}BOOST_SCOPE_EXIT_END;

		auto streamInfoIt = streamInfos_.find(streamID);
		if (streamInfoIt == streamInfos_.end())
		{
			ec = boost::system::error_code(boost::asio::error::connection_reset);
			return;
		}

		auto &streamInfo = streamInfoIt->second;
		auto itDest = buffers.begin();
		auto itSrc = messages.begin();
		
		std::size_t bytesDst = 0;

		while (itDest != buffers.end() && itSrc != messages.end())
		{
			Command command = itSrc->command;
			if (command == Command::transfer)
			{
				auto bufDst = *itDest + bytesDst;
				auto bufSrc = itSrc->data.toBuf();
				std::size_t size =
					std::min(boost::asio::buffer_size(bufDst), boost::asio::buffer_size(bufSrc));
				std::memcpy(
					boost::asio::buffer_cast<void*>(bufDst),
					boost::asio::buffer_cast<const void*>(bufSrc),
					size
					);
				sizeTotal += size;
				if (boost::asio::buffer_size(bufDst) == size)
				{
					++itDest;
					bytesDst = 0;
				}
				else
				{
					bytesDst += size;
				}
				if (boost::asio::buffer_size(bufSrc) == size)
				{
					itSrc = messages.erase(itSrc);
				}
				else
				{
					itSrc->data.consumed += size;
				}
			}
			else if (command == Command::eof)
			{
				ec = boost::system::error_code(boost::asio::error::eof);
				break;
			}
			else if (command == Command::close)
			{
				ec = boost::system::error_code(boost::asio::error::connection_aborted);
				break;
			}
			else if (command == Command::reset)
			{
				ec = boost::system::error_code(boost::asio::error::connection_reset);
				break;
			}
			else
			{
				break;
			}
		}
	};

	streamInfo.messages.pop(readHelper);
}

void PermanentConnection::go()
{
	createReadCoroutine();
	if (keepAlive_ && !keepAliveSet_)
	{
		createKeepAliveCoroutine();
	}
}

std::uint32_t PermanentConnection::newStreamID()
{
	do
	{
		streamIDCandidate_ += (active_ ? 1 : -1);
	} while (streamInfos_.find(streamIDCandidate_) != streamInfos_.end());
	return static_cast<std::uint32_t>(streamIDCandidate_);
}

void PermanentConnection::streamEnded(std::int32_t streamID)
{
	streamInfos_.erase(streamID);
}

PermanentConnection& PermanentConnection::operator << (std::tuple<const std::string&, std::shared_ptr<Svc> > arg)
{
	PermanentConnectionEntry* entry;
	const std::string& name = std::get<0>(arg);
	std::shared_ptr<Svc> svc = std::get<1>(arg);
	svc->insertEntry(std::unique_ptr < Entry > {
		entry = new PermanentConnectionEntry(name, shared_from_this())});

	virtualServices_.emplace(name, entry);
	return *this;
}

void PermanentConnection::removeVirtualService(const std::string& virtualService)
{
	virtualServices_.erase(virtualService);
}

void PermanentConnection::keepAlive(unsigned int v)
{
	keepAliveSet_ = true;
	if (keepAlive_)
	{
		keepAlive_ = v;
	}
	else if (v)
	{
		keepAlive_ = v;
		createKeepAliveCoroutine();
	}

	if (v == 0)
	{
		keepAliveTimer_.cancel();
	}
}

void PermanentConnection::certificate(X509* certificate, EVP_PKEY *pkey)
{
	if (::SSL_CTX_use_certificate(sslContext_->native_handle(), certificate) != 1)
	{
		throw boost::system::system_error(boost::asio::error::invalid_argument);
	}
	if (::SSL_CTX_use_PrivateKey(sslContext_->native_handle(), pkey) != 1)
	{
		throw boost::system::system_error(boost::asio::error::invalid_argument);
	}

	localPeerID_.certificate = PeerID::Certificate(true, certificate);
}

void PermanentConnection::ca(const std::vector<X509*>& ca)
{
	for (X509* x : ca)
	{
		::X509_STORE_add_cert(::SSL_CTX_get_cert_store(sslContext_->native_handle()), x);
	}
}

PermanentConnection::~PermanentConnection()
{
	assert(virtualServices_.empty());
	assert(streamInfos_.empty());
}
