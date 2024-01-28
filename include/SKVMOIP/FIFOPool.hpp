#pragma once

#include <SKVMOIP/defines.hpp>

#include <vector>
#include <deque>
#include <algorithm>

#include <bufferlib/buffer.h>

namespace SKVMOIP
{
	class DataBuffer
	{
	private:
		buffer_t* m_buffer;
		bool m_isValid;
	
	public:
		DataBuffer() : m_buffer(NULL), m_isValid(false) { }
		DataBuffer(u32 capacity);
		~DataBuffer() { m_isValid = false; m_buffer = NULL; }

		void destroy();
	
		const u8* getPtr() const;
		u8* getPtr();
		u32 getSize() const;
	};	

	template<typename T>
	class FIFOPool
	{
	public:
		typedef typename std::vector<T>::size_type ItemIdType;
		typedef Pair<T, ItemIdType> ItemType;
		typedef Optional<typename FIFOPool<T>::ItemType> PoolItemType;
	private:
		std::vector<T> m_buffer;
	
		std::deque<ItemIdType> m_activeQueue;
		std::vector<ItemIdType> m_inactiveQueue;
		void (*m_destroyCallback)(T&);
	
	public:

		static OptionalReference<T> GetValue(PoolItemType& itemType)
		{
			if(!itemType) return { };
			return { itemType->first };
		}

		FIFOPool(void (*destroyCallback)(T&));
		FIFOPool(FIFOPool&& pool);
		FIFOPool& operator=(FIFOPool&& pool);
		FIFOPool(FIFOPool& pool) = delete;
		FIFOPool& operator=(FIFOPool& pool) = delete;
		~FIFOPool();
	
		ItemIdType getCount() const { return m_buffer.size(); }

		PoolItemType getActive();
		bool hasActive() const { return !m_activeQueue.empty(); }
		void returnActive(PoolItemType item);
		PoolItemType getInactive();
		bool hasInactive() const { return !m_inactiveQueue.empty(); }
		void returnInactive(PoolItemType item);
		template<typename... Args>
		void createInactive(Args... args);

		void setDestroyCallback(void (*callback)(T&)) { m_destroyCallback = callback; }
	};

	template<typename T>
	FIFOPool<T>::FIFOPool(void (*destroyCallback)(T&)) : m_destroyCallback(destroyCallback) { }
	template<typename T>
	FIFOPool<T>::FIFOPool(FIFOPool&& pool) : 
											m_buffer(std::move(pool.m_buffer)),
											m_activeQueue(std::move(pool.m_activeQueue)),
											m_inactiveQueue(std::move(pool.m_inactiveQueue)),
											m_destroyCallback(pool.m_destroyCallback)
	{
	
	}
	template<typename T>
	FIFOPool<T>& FIFOPool<T>::operator=(FIFOPool&& pool)
	{
		m_buffer = std::move(pool.m_buffer);
		m_activeQueue = std::move(pool.m_activeQueue);
		m_inactiveQueue = std::move(pool.m_inactiveQueue);
		m_destroyCallback = pool.m_destroyCallback;
		return *this;
	}

	template<typename T>
	FIFOPool<T>::~FIFOPool()
	{
		if(m_destroyCallback != NULL)
			std::for_each(m_buffer.begin(), m_buffer.end(), [this](T& v) { m_destroyCallback(v); });
	}
	
	template<typename T>
	typename FIFOPool<T>::PoolItemType FIFOPool<T>::getActive()
	{
		if(m_activeQueue.empty())
			return { };
		ItemIdType id = m_activeQueue.back();
		m_activeQueue.pop_back();
		return { ItemType(m_buffer[id], id) };
	}
	
	template<typename T>
	void FIFOPool<T>::returnActive(PoolItemType item)
	{
		if(!item) return;
		_assert(item->second < m_buffer.size());
		m_inactiveQueue.push_back(item->second);
	}
	
	template<typename T>
	typename FIFOPool<T>::PoolItemType FIFOPool<T>::getInactive()
	{
		if(m_inactiveQueue.empty())
			return { };
		ItemIdType id = m_inactiveQueue.back();
		m_inactiveQueue.pop_back();
		return { ItemType(m_buffer[id], id) };
	}

	template<typename T>
	void FIFOPool<T>::returnInactive(PoolItemType item)
	{
		if(!item) return;
		_assert(item->second < m_buffer.size());
		m_activeQueue.push_front(item->second);
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