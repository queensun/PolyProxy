#pragma once
#include "static.h"
#include "Stream.h"
#include "Svc.h"
#include "InterCoroutineVar.h"
#include "SslContext.h"

class PermanentConnectionEntry;
class PermanentConnection
	: public std::enable_shared_from_this<PermanentConnection>
{
public:
	PermanentConnection(
		std::unique_ptr<Stream>&& stream,
		bool active);
	~PermanentConnection();
	void sendKeepAlive(boost::asio::yield_context yield);
	std::unique_ptr<Stream> open(
		const std::string& virtualService, boost::asio::yield_context yield);
	bool sendCertificate(boost::asio::yield_context yield);
	void requestCertificate(boost::asio::yield_context yield);
	inline unsigned int keepAlive() const
	{
		return keepAlive_;
	}
	void keepAlive(unsigned int v);
	
	//called by a stream object when it is dead
	void streamEnded(std::int32_t streamID);

	PermanentConnection& operator << (std::tuple<const std::string&, std::shared_ptr<Svc> > arg);
	void removeVirtualService(const std::string& virtualService);

	void go();
	inline bool active() const
	{
		return active_;
	}
	inline std::size_t maxStreams() const
	{
		return maxStreams_;
	}
	inline void maxStreams(std::size_t v) 
	{
		maxStreams_ = v;
	}

	enum class Command
	{
		keepAlive = 0,
		open,
		close,
		transfer,
		opened,
		refused,
		eof,
		reset,
		sendCertificate,
		requestCertificate,
		count
	};

	enum
	{
		HEADER_SIZE=8
	};

	struct StreamInfo
	{
	public:
		friend class PermanentConnection;

	private:
		struct Message
		{
			Command command;
			struct Data
				: public std::vector < char >
			{
				std::size_t consumed = 0;
				boost::asio::const_buffer toBuf() const
				{
					return boost::asio::const_buffer(
						data() + consumed,
						size() - consumed
						);
				}

				Data()
				{}

				Data(std::vector < char >&& o)
					: std::vector < char >(std::move(o))
				{}

				Data(Data&& o)
					: std::vector < char >(std::move(o))
					, consumed(o.consumed)
				{}

				Data& operator=(Data&& o)
				{
					operator = (std::move(o));
					consumed = o.consumed;
				}
			};
			Data data;
		};
		InterCoroutineVar<Message> messages;
	};


	std::size_t write(
		const ConstBuffers & buffers,
		boost::asio::yield_context yield);
	void write(
		const ConstBuffers & buffers,
		const Stream::WriteHandler& handler);
	std::size_t write(
		Command command,
		std::uint32_t streamID,
		const ConstBuffers & buffers,
		boost::asio::yield_context yield);
	void write(
		Command command,
		std::uint32_t streamID,
		const ConstBuffers & buffers,
		const Stream::WriteHandler& handler);

	StreamInfo::Message read(
		StreamInfo& streamInfo,
		boost::asio::yield_context yield);
	void read(
		std::int32_t streamID,
		StreamInfo& streamInfo,
		const MutableBuffers& buffers,
		const Stream::ReadHandler& readHandler);

	using ReconnectFunc = std::function < void(void) > ;
	void reconnectFunc(const ReconnectFunc& func)
	{
		reconnectFunc_ = func;
	}
	inline bool valid() const
	{
		return !zombie_;
	}

	void certificate(X509* certificate, EVP_PKEY *pkey);
	void ca(const std::vector<X509*>& ca);

	const PeerID& remotePeerID() const
	{
		return remotePeerID_;
	}
	const PeerID& localPeerID() const
	{
		return localPeerID_;
	}

private:
	std::unique_ptr<Stream> stream_;
	PeerID localPeerID_, remotePeerID_;
	bool keepAliveSet_ = false;
	unsigned int keepAlive_ = 60;
	boost::asio::steady_timer keepAliveTimer_{ ios };
	boost::asio::steady_timer writeTimer_{ ios, boost::asio::steady_timer::clock_type::duration::max() };
	bool writing_ = false;

	ReconnectFunc reconnectFunc_;
	std::unique_ptr<SslContext> sslContext_;
	struct WritingBuffer
	{
		std::array<char, 2> command;
		std::array<char, 4> streamID;
		std::array<char, 2> len;
	};
	 
	std::tuple<std::shared_ptr<WritingBuffer>, std::vector<boost::asio::const_buffer> >
		fillInWriteBuffer(
		Command command,
		std::uint32_t streamID,
		const ConstBuffers & buffers);
	bool active_;
	bool zombie_ = false;
	using StreamInfos = std::map < std::uint32_t, StreamInfo>;
	StreamInfos streamInfos_;
	using VirtualServices = 
		std::map < std::string, PermanentConnectionEntry* > ;
	VirtualServices virtualServices_;
	std::int32_t streamIDCandidate_ = 0;
	std::size_t maxStreams_ = 1024 * 1024;

	void createReadCoroutine();
	void createKeepAliveCoroutine();
	void end(boost::asio::yield_context yield);
	std::uint32_t newStreamID();

	enum
	{
		MAX_TRANSFER_SIZE = (1 << 16) - 1
	};
};