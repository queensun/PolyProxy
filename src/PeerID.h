#include "static.h"

struct PeerID
{
	boost::asio::ip::address address;
	std::uint16_t port=0;

	class Certificate
	{
	public:
		inline Certificate()
			: x509_(nullptr)
		{}
		inline Certificate(bool valid, X509* x509)
			: valid_(valid)
			, x509_(X509_dup(x509))
		{}
		inline Certificate(Certificate&& o)
		{
			valid_ = o.valid_;
			x509_ = o.x509_;
			o.x509_ = nullptr;
		}
		inline Certificate(const Certificate& o)
		{
			valid_ = o.valid_;
			x509_ = X509_dup(o.x509_);
		}
		inline Certificate& operator =(Certificate&& o)
		{
			valid_ = o.valid_;
			x509_ = o.x509_;
			o.x509_ = nullptr;
			return *this;
		}
		inline Certificate& operator =(Certificate& o)
		{
			valid_ = o.valid_;
			x509_ = X509_dup(o.x509_);
			return *this;
		}
		inline ~Certificate()
		{
			if (x509_)
				::X509_free(x509_);
		}
		inline operator bool()const
		{
			return x509_ != nullptr;
		}
		inline bool valid() const
		{
			return valid_;
		}

		bool checkWebHostName(const std::string& hostname) const;
		bool checkAltNames(const std::function<bool(char const*, std::size_t)> func) const;
		bool checkAltNames(boost::asio::ip::address& address) const;
		bool checkCommonNames(const std::function<bool(char const*, std::size_t)> func) const;
		bool checkCommonNames(const boost::asio::ip::address& address) const;

	private:
		bool valid_ = false;
		X509* x509_;
	} certificate;

	std::string virtualService;

	PeerID()
	{}

	PeerID(const boost::asio::ip::tcp::endpoint& endPoint)
		: address(endPoint.address()), port(endPoint.port())
	{}
};