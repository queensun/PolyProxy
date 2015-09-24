#pragma once
#include <map>
#include <memory>

template <typename Key, typename Value, typename Child>
class WeakPtrMap
	: protected std::map < Key, std::weak_ptr<Value> >
	, public std::enable_shared_from_this<Child>
{
public:
	using Base = std::map < Key, std::weak_ptr<Value> >;
	inline bool empty() const
	{
		return Base::empty();
	}
	inline size_type size() const
	{
		return Base::size();
	}
	inline size_type erase(const Key& k)
	{
		return Base::erase(k);
	}
	std::shared_ptr<Value> operator[] (const Key& k)
	{
		auto found = Base::find(k);
		if (found == Base::end())
		{
			std::shared_ptr<Value> ret = 
				std::make_shared<Value>(shared_from_this(), k);
			Base::insert(
				std::make_pair(k, std::weak_ptr<Value>(ret)));
			return ret;
		}
		else
		{
			return found->second.lock();
		}
	}
};