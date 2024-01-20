#pragma once

#include <SKVMOIP/defines.hpp>

#include <vector>
#include <deque>
#include <optional>

namespace SKVMOIP
{
	template<typename T>
	class FIFOPool
	{
	public:
		typedef typename std::vector<T>::size_type ItemIdType;
		typedef std::pair<T&, ItemIdType> ItemType;
	private:
		std::vector<T> m_buffer;
	
		std::deque<ItemIdType> m_activeQueue;
		std::vector<ItemIdType> m_inactiveQueue;
	
	public:
		FIFOPool();
		FIFOPool(FIFOPool&& pool);
		FIFOPool& operator=(FIFOPool&& pool);
		FIFOPool(FIFOPool& pool) = delete;
		FIFOPool& operator=(FIFOPool& pool) = delete;
		~FIFOPool();
	
		std::optional<ItemType> getActive();
		void returnActive(ItemType value);
		std::optional<ItemType> getInactive();
		void returnInactive(ItemType value);
		template<typename... Args>
		void createInactive(Args... args);
	};

	template<typename T>
	FIFOPool<T>::FIFOPool() { }
	template<typename T>
	FIFOPool<T>::FIFOPool(FIFOPool&& pool) : 
											m_buffer(std::move(pool.m_buffer)),
											m_activeQueue(std::move(pool.m_activeQueue)),
											m_inactiveQueue(std::move(pool.m_inactiveQueue))
	{
	
	}
	template<typename T>
	FIFOPool<T>& FIFOPool<T>::operator=(FIFOPool&& pool)
	{
		m_buffer = std::move(pool.m_buffer);
		m_activeQueue = std::move(pool.m_activeQueue);
		m_inactiveQueue = std::move(pool.m_inactiveQueue);
		return *this;
	}

	template<typename T>
	FIFOPool<T>::~FIFOPool() { }
	
	template<typename T>
	std::optional<typename FIFOPool<T>::ItemType> FIFOPool<T>::getActive()
	{
		if(m_activeQueue.empty())
			return { };
		ItemIdType id = m_activeQueue.back();
		m_activeQueue.pop_back();
		return { ItemType(m_buffer[id], id) };
	}
	
	template<typename T>
	void FIFOPool<T>::returnActive(ItemType value)
	{
		_assert(value.second < m_buffer.size());
		m_inactiveQueue.push_back(value.second);
	}
	
	template<typename T>
	std::optional<typename FIFOPool<T>::ItemType> FIFOPool<T>::getInactive()
	{
		if(m_inactiveQueue.empty())
			return { };
		ItemIdType id = m_inactiveQueue.back();
		m_inactiveQueue.pop_back();
		return { ItemType(m_buffer[id], id) };
	}
	
	template<typename T>
	void FIFOPool<T>::returnInactive(ItemType value)
	{
		_assert(value.second < m_buffer.size());
		m_activeQueue.push_front(value.second);
	}

	template<typename T>
	template<typename... Args>
	void FIFOPool<T>::createInactive(Args... args)
	{
		T v(std::forward<Args>(args)...);
		m_inactiveQueue.push_back(m_buffer.size());
		m_buffer.push_back(std::move(v));
	}
}