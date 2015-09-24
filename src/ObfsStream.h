#pragma once
#include "static.h"
#include "TcpStream.h"
#include "Obfs.h"

template <typename Base=TcpStream>
class ObfsStream
	: public Base
{
public:
	ObfsStream(Base &&o, Obfs&& obfsRead, Obfs&& obfsWrite)
		: Base(std::move(o))
		, obfsRead_(std::move(obfsRead))
		, obfsWrite_(std::move(obfsWrite))
	{}
	template <typename Arg>
	ObfsStream(Arg &&arg, Obfs&& obfsRead, Obfs&& obfsWrite)
		: Base(std::move(arg))
		, obfsRead_(std::move(obfsRead))
		, obfsWrite_(std::move(obfsWrite))
	{}
	ObfsStream(ObfsStream &&o)
		: Base(std::move(o))
		, obfsRead_(std::move(o.obfsRead_))
		, obfsWrite_(std::move(o.obfsWrite_))
	{
	}

	using ReadHandler = typename Base::ReadHandler;
	using WriteHandler = typename Base::WriteHandler;

	virtual void readSome(const MutableBuffers& buffers, const ReadHandler& handler);
	virtual void writeSome(const ConstBuffers& buffers, const WriteHandler& handler);

protected:
	Obfs obfsRead_, obfsWrite_;
	std::vector<char> writeBuf_;
	
	class SecBytesQueue
	{
	public:
		inline void add(std::vector<char> &&v)
		{
			std::size_t size = v.size();
			data_.emplace_back(std::move(v));
			size_ += size;
		}

		inline void consume(std::size_t bytes)
		{
			consumed_ += bytes;
			while (consumed_ > 0 && consumed_ >= data_.front().size())
			{
				consumed_ -= data_.front().size();
				data_.pop_front();
			}
			size_ -= bytes;
		}
		inline std::size_t size() const
		{
			return size_;
		}
		struct const_iterator
			: public std::tuple < std::vector<char>::const_iterator, std::vector<char>::const_iterator, std::list<std::vector<char> >::const_iterator >
			//					iterator								end								iterator
			, public std::iterator < std::forward_iterator_tag, char >
		{
			using ItBase = 
				std::tuple < std::vector<char>::const_iterator, std::vector<char>::const_iterator, std::list<std::vector<char> >::const_iterator > ;
			const_iterator(const ItBase& b)
				: ItBase(b)
			{}

			const_iterator operator ++()
			{
				ItBase * _ = this;
				auto &zero = std::get<0>(*_);
				auto &one = std::get<1>(*_);
				auto &two = std::get<2>(*_);
				if (++zero == one)
				{
					++two;
					zero = two->begin();
					one = two->end();
				}
				return *this;
			}
			const char& operator *() const
			{
				const ItBase * _ = this;
				auto &zero = std::get<0>(*_);
				return *zero;
			}
		};

		const_iterator cbegin() const
		{
			return std::make_tuple(data_.cbegin()->cbegin() + consumed_, data_.cbegin()->cend(), data_.cbegin());
		}

	private:
		std::list<std::vector<char> > data_;
		std::size_t size_ = 0;
		std::size_t consumed_ = 0;
	};

	SecBytesQueue secBytesQueue_;
};