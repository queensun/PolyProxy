#pragma once
#include "static.h"

class MutableBuffers
	: public std::vector < boost::asio::mutable_buffer >
{
public:
	using Base = std::vector < boost::asio::mutable_buffer > ;

	template <typename T>
	MutableBuffers(const T& t)
		: Base(t.begin(), t.end())
	{
	}

	MutableBuffers(const boost::asio::mutable_buffer& o)		
	{
		push_back(o);
	}
};

class MutableOwnedBuffers
	: public std::vector < boost::asio::mutable_buffer >
{
public:
	using Base = std::vector < boost::asio::mutable_buffer >;

	MutableOwnedBuffers(MutableOwnedBuffers&& o)
		: Base(std::move(o))
		, bufs_(std::move(o.bufs_))
		, lastBufferSize_(o.lastBufferSize_)
	{
		o.lastBufferSize_ = 0;
		o.bufs_.clear();
		o.clear();
	}

	MutableOwnedBuffers& operator= (MutableOwnedBuffers&& o)
	{
		Base::operator = (std::move(o));
		bufs_ = std::move(o.bufs_);
		lastBufferSize_ = o.lastBufferSize_;

		o.lastBufferSize_ = 0;
		o.bufs_.clear();
		o.clear();

		return *this;
	}

	MutableOwnedBuffers(std::size_t n)
	{
		increase(n);
	}

	MutableOwnedBuffers()
	{}

	void increase(std::size_t n)
	{
		bufs_.emplace_back(std::vector<char>(n));
		emplace_back(boost::asio::buffer(bufs_.back()));
		lastBufferSize_ = 0;
	}

	inline void lastTransferred(std::size_t n)
	{
		lastBufferSize_ += n;
	}

	inline std::size_t lastBufferSize() const
	{
		return lastBufferSize_;
	}

	void append(MutableOwnedBuffers&& o)
	{
		if (!empty())
		{
			pop_back();
			emplace_back(
				boost::asio::buffer(bufs_.back().data(), lastBufferSize_)
				);
		}

		for (auto i = o.bufs_.begin(); i != o.bufs_.end(); ++i)
		{
			bufs_.emplace_back(std::move(*i));
			emplace_back(boost::asio::buffer(bufs_.back()));
		}

		lastBufferSize_ = o.lastBufferSize_;

		o.lastBufferSize_ = 0;
		o.bufs_.clear();
		o.clear();
	}

	std::size_t size() const
	{
		if (Base::empty())
			return 0;

		std::size_t ret = 0;
		std::for_each(begin(), end()-1, 
			[&ret](boost::asio::mutable_buffer buf)
		{
			ret += boost::asio::buffer_size(buf);
		});

		return ret + lastBufferSize_;
	}

	bool empty() const
	{
		return size() == 0;
	}

	std::vector<std::vector<char> > release()
	{
		std::vector<std::vector<char> > ret(std::move(bufs_));
		if (!ret.empty())
		{
			ret.back().resize(lastBufferSize_);
		}

		bufs_.clear();
		lastBufferSize_ = 0;

		return ret;
	}

private:
	std::vector<std::vector<char> > bufs_;
	std::size_t lastBufferSize_ = 0;
};

class ConstBuffers
	: public std::vector < boost::asio::const_buffer >
{
public:
	using Base = std::vector < boost::asio::const_buffer >;

	template <typename T>
	ConstBuffers(const T& t)
		: Base(t.begin(), t.end())
	{
	}

	ConstBuffers(const MutableOwnedBuffers& o)
		: Base(o.cbegin(), (o.end()-1))
	{
		auto b = *o.crbegin();
		push_back(boost::asio::buffer(
			boost::asio::buffer_cast<char const*>(b),
			o.lastBufferSize()
			));
	}

	ConstBuffers()
	{}
};

class ConstStringFromBuffers
	:protected ConstBuffers
{
public:
	ConstStringFromBuffers()
	{}

	ConstStringFromBuffers(const ConstBuffers& buffers)
		: ConstBuffers(buffers)
	{}
	//template <typename T>
	//ConstStringFromBuffers(const T& t)
	//	: ConstBuffers(t)
	//{
	//}

	class const_iterator
		: public std::tuple < ConstBuffers::const_iterator, ConstBuffers::const_iterator, char const* >
		//					iterator						end								iterator
		, public std::iterator<std::forward_iterator_tag, char>
	{
	public:
		using Base = std::tuple < ConstBuffers::const_iterator, ConstBuffers::const_iterator, char const* >;
		const_iterator(ConstBuffers::const_iterator zero, ConstBuffers::const_iterator one, char const* two)
			: std::tuple < ConstBuffers::const_iterator, ConstBuffers::const_iterator, char const* >(zero, one, two)
		{}
		const_iterator operator ++()
		{
			Base * _ = this;
			auto &zero = std::get<0>(*_);
			auto &one = std::get<1>(*_);
			auto &two = std::get<2>(*_);

			if (boost::asio::buffer_cast<char const*>(*zero) + boost::asio::buffer_size(*zero) == ++two)
			{
				++zero;
				if (zero != one)
				{
					two = boost::asio::buffer_cast<char const*>(*zero);
				}
				else
				{
					two = nullptr;
				}
			}

			return *this;
		}

		bool operator == (const const_iterator& o)
		{
			return std::get<0>(Base(*this)) == std::get<0>(Base(o))
				&& std::get<2>(Base(*this)) == std::get<2>(Base(o))
				;
		}

		bool operator != (const const_iterator& o)
		{
			return std::get<0>(Base(*this)) != std::get<0>(Base(o))
				|| std::get<2>(Base(*this)) != std::get<2>(Base(o))
				;
		}

		const char& operator *()
		{
			return *std::get<2>(Base(*this));
		}
	};

	class const_reverse_iterator
		: public std::tuple < ConstBuffers::const_reverse_iterator, ConstBuffers::const_reverse_iterator, char const* >
		//						iterator							end										iterator
		, public std::iterator<std::forward_iterator_tag, char>
	{
	public:
		using Base = std::tuple < ConstBuffers::const_reverse_iterator, ConstBuffers::const_reverse_iterator, char const* >;
		const_reverse_iterator(ConstBuffers::const_reverse_iterator zero, ConstBuffers::const_reverse_iterator one, char const* two)
			: std::tuple < ConstBuffers::const_reverse_iterator, ConstBuffers::const_reverse_iterator, char const* >(zero, one, two)
		{}
		const_reverse_iterator operator ++()
		{
			Base * _ = this;
			auto &zero = std::get<0>(*_);
			auto &one = std::get<1>(*_);
			auto &two = std::get<2>(*_);

			if (boost::asio::buffer_cast<char const*>(*zero) < two)
			{
				--two;
			}
			else
			{
				zero++;
				if (zero != one)
				{
					two = boost::asio::buffer_cast<char const*>(*zero) + boost::asio::buffer_size(*zero) - 1;
				}
				else
				{
					two = nullptr;
				}
			}

			return *this;
		}
		bool operator != (const const_reverse_iterator& o)
		{
			return std::get<0>(Base(*this)) != std::get<0>(Base(o))
				|| std::get<2>(Base(*this)) != std::get<2>(Base(o))
				;
		}

		const char& operator *()
		{
			return *std::get<2>(Base(*this));
		}
	};

	const_iterator begin() const
	{
		ConstBuffers::const_iterator zero = ConstBuffers::begin();
		return const_iterator(zero, ConstBuffers::end(),
			boost::asio::buffer_cast<char const*>(*zero));
	}

	const_iterator end() const
	{
		return const_iterator(ConstBuffers::end(), ConstBuffers::end(), nullptr);
	}

	const_iterator cbegin() const
	{
		return begin();
	}

	const_iterator cend() const
	{
		return end();
	}

	const_reverse_iterator rbegin() const
	{
		ConstBuffers::const_reverse_iterator zero = ConstBuffers::rbegin();
		return const_reverse_iterator(zero, ConstBuffers::rend(),
			boost::asio::buffer_cast<char const*>(*zero) + boost::asio::buffer_size(*zero) - 1);
	}

	const_reverse_iterator rend() const
	{
		return const_reverse_iterator(ConstBuffers::rend(), ConstBuffers::rend(), nullptr);
	}

	const_reverse_iterator crbegin() const
	{
		return rbegin();
	}

	const_reverse_iterator crend() const
	{
		return rend();
	}

	std::size_t size() const
	{
		std::size_t n = 0;
		std::for_each(ConstBuffers::begin(), ConstBuffers::end(),
			[&n](const boost::asio::const_buffer &b)
		{
			n += boost::asio::buffer_size(b);
		});
		return n;
	}

	inline std::size_t length() const
	{
		return size();
	}

	inline bool empty() const
	{		
		return size()==0;
	}

	char front() const
	{
		auto t = ConstBuffers::front();
		return *boost::asio::buffer_cast<char const*>(t);
	}

	char back() const
	{
		auto t = ConstBuffers::back();
		return *(boost::asio::buffer_cast<char const*>(t)+(boost::asio::buffer_size(t) - 1));
	}

	inline const char& operator[] (size_t pos) const
	{
		char const* ret = atPtr(pos);
		return *ret;
	}

	inline const char& at(size_t pos) const
	{
		char const* ret = atPtr(pos);
		if (!ret)
		{
			throw std::out_of_range("out of range");
		}
		return *ret;
	}

	size_t copy(char* s, size_t len, size_t pos = 0) const
	{
		char const* src = nullptr;
		ConstBuffers::const_iterator it;

		std::tie(it, src) = atLowLevelPtr(pos);

		if (!src)
		{
			throw std::out_of_range("out of range");
		}

		std::size_t i = 0;

		for (; i<len; ++len)
		{
			s[i] = *src++;
			char const* bufBegin = boost::asio::buffer_cast<char const*>(*it);
			std::size_t bufSize = boost::asio::buffer_size(*it);
			char const* bufEnd = bufBegin + bufSize;

			if (src == bufEnd)
			{
				if (++it != ConstBuffers::end())
				{
					src = boost::asio::buffer_cast<char const*>(*it);
				}
				else
				{
					break;
				}
			}
		}

		return i;
	}

	inline ConstStringFromBuffers substr(const_iterator i) const
	{
		return substr(i, end());
	}

	ConstStringFromBuffers substr(const_iterator i, const_iterator e) const
	{
		ConstStringFromBuffers ret;
		ret.reserve(ConstBuffers::size());
		
		const_iterator::Base &i_ = i;
		const_iterator::Base &e_ = e;
		if (std::get<0>(i_) == std::get<0>(e_))
		{
			if (std::get<2>(e_)-std::get<2>(i_))
			{
				ret.push_back(boost::asio::buffer(
					std::get<2>(i_), std::get<2>(e_)-std::get<2>(i_)));
			}
		}
		else
		{
			{
				boost::asio::const_buffer buffer = *std::get<0>(i_);
				char const* bufferBegin = boost::asio::buffer_cast<char const*>(buffer);
				std::size_t bufferSize = boost::asio::buffer_size(buffer);
				char const* newBufferBegin = std::get<2>(i_);
				std::size_t newBufferSize = bufferBegin + bufferSize - newBufferBegin;
				ret.push_back(boost::asio::buffer(newBufferBegin, newBufferSize));
			}

			for (auto it = std::get<0>(i_)+1; 
				it != std::get<0>(e_); ++it)
			{
				ret.push_back(*it);
			}

			if (std::get<0>(e_) != std::get<1>(e_))
			{
				boost::asio::const_buffer buffer = *std::get<0>(e_);
				char const* bufferBegin = boost::asio::buffer_cast<char const*>(buffer);
				char const* bufferEnd = std::get<2>(e_);
				std::size_t newBufferSize = bufferEnd - bufferBegin;
				ret.push_back(boost::asio::buffer(bufferBegin, newBufferSize));
			}
		}

		return ret;
	}

	ConstStringFromBuffers substr(size_t pos = 0, size_t len = npos) const
	{
		ConstBuffers::const_iterator itBegin;
		char const* cBegin;
		std::tie(itBegin, cBegin) = atLowLevelPtr(pos);

		ConstStringFromBuffers ret;
		ret.reserve(ConstBuffers::size());

		std::size_t firstBufSize = 
			boost::asio::buffer_cast<char const*>(*itBegin) + boost::asio::buffer_size(*itBegin) - cBegin;

		if (len < firstBufSize)
		{
			ret.push_back(boost::asio::buffer(cBegin, len));
			return ret;
		}
		else
		{
			ret.push_back(boost::asio::buffer(cBegin, firstBufSize));
			len = (len==npos?npos:len-firstBufSize);
		}

		std::find_if(itBegin + 1, ConstBuffers::end(),
			[&len, &ret](const boost::asio::const_buffer& buf) -> bool
		{
			char const* bufData = boost::asio::buffer_cast<char const*>(buf);
			std::size_t bufSize = boost::asio::buffer_size(buf);
			if (len < bufSize)
			{
				ret.push_back(boost::asio::buffer(bufData, len));
				return true;
			}
			else
			{
				ret.push_back(boost::asio::buffer(bufData, bufSize));
				len = (len == npos ? npos : len - bufSize);
				return false;
			}			
		}
			);

		return ret;
	}

	int compare(const std::string& str)
	{
		int ret=0;

		const_iterator i = begin();
		const_iterator e = end();

		std::string::const_iterator is = str.begin();
		std::string::const_iterator es = str.end();
		for (; i != e && is!=es; ++i, ++is)
		{
			if (*i != *is)
				return *i - *is;
		}

		if (i == e && is == es)
		{
			return 0;
		}
		else if (is == es)
		{
			return 1;
		}
		else/*if (i==e)*/
		{
			return -1;
		}
	}

	int compare(const char* s)
	{
		const_iterator it = cbegin();
		const_iterator itEnd = cend();
		std::size_t i = 0;
		std::size_t tlen = strlen(s);

		for (; it != itEnd && i < tlen; ++it, ++i)
		{
			if (*it != s[i])
			{
				return *it - s[i];
			}
		}

		if (i == tlen && it == itEnd)
		{
			return 0;
		}
		else if (i == tlen)
		{
			return 1;
		}
		else/*if (it == itEnd)*/
		{
			return -1;
		}
	}

	bool operator== (const std::string& str)
	{
		return compare(str) == 0;
	}

	bool operator== (char const* str)
	{
		return compare(str) == 0;
	}

	bool operator!= (const std::string& str)
	{
		return compare(str) != 0;
	}

	bool operator!= (char const* str)
	{
		return compare(str) != 0;
	}

	ConstStringFromBuffers rtrim(char c) const
	{
		if (empty())
		{
			return ConstBuffers();
		}

		ConstStringFromBuffers ret(*this);
		boost::asio::const_buffer buf = ret.Base::back();
		char const* bufBegin = boost::asio::buffer_cast<char const*>(buf);
		std::size_t bufSize = boost::asio::buffer_size(buf);

		if (bufBegin[bufSize-1]==c)
		{
			ret.Base::pop_back();
			if (bufSize > 1)
			{
				ret.Base::push_back(boost::asio::buffer(bufBegin, bufSize-1));
			}
			return ret;
		}
		else
		{
			return *this;
		}
	}

	ConstStringFromBuffers chomp() const
	{
		return rtrim('\n').rtrim('\r');
	}

	ConstStringFromBuffers operator +(const ConstStringFromBuffers& o)
	{
		ConstStringFromBuffers ret(*this);
		ret.Base::insert(ret.Base::end(), o.Base::begin(), o.Base::end());
		return ret;
	}
	enum
	{
		npos=-1
	};
private:
	std::tuple<ConstBuffers::const_iterator, char const*> atLowLevelPtr(size_t pos) const
	{
		char const* ptr = nullptr;
		ConstBuffers::const_iterator 
			it = std::find_if(ConstBuffers::begin(), ConstBuffers::end(),
			[&pos, &ptr](const boost::asio::const_buffer &b) -> bool
		{
			std::size_t length = boost::asio::buffer_size(b);
			if (pos < length)
			{
				ptr = boost::asio::buffer_cast<char const*>(b)+pos;
				return true;
			}
			else
			{
				pos -= length;
				return false;
			}
		});

		return std::make_tuple(it, ptr);
	}

	char const* atPtr(size_t pos) const
	{
		char const* ptr;
		std::tie(std::ignore, ptr) = atLowLevelPtr(pos);
		return ptr;
	}
};

template <typename T>
T constStringCast(const ConstStringFromBuffers& b);

template <typename T, typename Buffers>
inline T buffersCast(const Buffers& b)
{ 
	return constStringCast<T>(ConstStringFromBuffers(b));
}

template <typename T>
inline T constStringCast(const ConstStringFromBuffers& b)
{
	T ret = 0;
	auto it = b.begin();
	for (std::size_t i = 0; i < sizeof(T); ++i, ++it)
	{
		ret <<= 8;
		ret |= static_cast<unsigned char>(*it);
	}
	return ret;
}


template <>
inline unsigned char constStringCast(const ConstStringFromBuffers& b)
{
	return static_cast<unsigned char>(b[0]);
}

template <>
inline std::string constStringCast(const ConstStringFromBuffers& b)
{
	std::string ret;
	if (b.empty())
	{
		return ret;
	}
		
	ret.resize(b.size());
	std::copy(b.begin(), b.end(), ret.begin());
	return ret;
}

template <typename T, std::size_t size>
void constStringCastToSequence(const ConstStringFromBuffers& b, std::array<T, size>& sequence);

template <typename Buffers, typename T, std::size_t size>
inline void buffersCastToSequence(const Buffers& b, std::array<T, size>& sequence)
{
	return constStringCastToSequence<T>(ConstStringFromBuffers(b), sequence);
}

template <typename T, std::size_t size>
inline void constStringCastToSequence(const ConstStringFromBuffers& b, std::array<T, size>& sequence)
{
	auto it=b.begin();
	for (std::size_t i = 0; i < size; ++i, ++it)
	{
		sequence[i] = *it;
	}
}