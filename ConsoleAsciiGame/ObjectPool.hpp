#pragma once
#include <vector>
#include <functional>
#include "Debug.hpp"

template<typename T>
using PoolCollection = std::vector<T>;

template<typename T>
class ObjectPool
{
private:
	//TODO: this could probably be optimized (maybe with array since we know max size)?
	PoolCollection<T> m_pool;
	/// <summary>
	/// Stores the index of the last USABLE element. -1 if there are no used elements 
	/// (either because all are unused or there are no elements in the first place)
	/// </summary>
	size_t m_usedEndIndex;

public:

private:
	T& GetAtUnsafe(const size_t& index)
	{
		return m_pool[index];
	}

public:
	ObjectPool(const size_t& maxSize) : m_pool(), m_usedEndIndex(-1)
	{ 
		m_pool.reserve(maxSize);
	}

	bool TryReserveNewSize(const size_t& newSize)
	{
		if (!Assert(this, m_pool.empty(), std::format("Tried to reserve a new size for object pool:{} after initial construction "
			"of size:{} but elements are already added so no new size can be set", std::to_string(newSize), std::to_string(GetMaxCapacity()))))
			return false;

		m_pool.reserve(newSize);
		return true;
	}

	T* TryAdd(const T& element)
	{
		if (!Assert(this, !IsAtCapacity(), std::format("Tried to add a new object of type:{} to pool but "
			"max capacity:{} has been reached", Utils::GetTypeName<T>(), std::to_string(GetMaxCapacity()))))
			return nullptr;

		T* result = nullptr;
		if (HasUnusedAvailable())
		{
			//Since index is last USED, +1 is the next available
			m_pool[m_usedEndIndex + 1] = element;
			result = &(m_pool[m_usedEndIndex + 1]);
		}
		else
		{
			result = &(m_pool.emplace_back(element));
		}
		m_usedEndIndex++;
		
		return result;
	}

	T& GetAt(const size_t& index)
	{
		if (!Assert(this, m_usedEndIndex != -1 && 0 <= index && index <= m_usedEndIndex, 
			std::format("Tried to get object at index:{} of pool but it is out of bounds of used space:[0,{}]", 
				std::to_string(index), std::to_string(m_usedEndIndex))))
			throw std::invalid_argument("Invalid pool index");

		return GetAtUnsafe(index);
	}

	void SetUnused(const size_t& index)
	{
		if (!Assert(this, m_usedEndIndex !=-1 && 0 <= index && index <= m_usedEndIndex, 
			std::format("Tried to set object at index:{} of pool to unused "
			"but it is out of bounds of used space:[0,{}]", std::to_string(index), std::to_string(m_usedEndIndex))))
			throw std::invalid_argument("Invalid pool index");

		//If we want to set the last usavble index to unusable we do not need to swap
		if (index != m_usedEndIndex)
		{
			T currentBackElement = m_pool[m_usedEndIndex];
			m_pool[m_usedEndIndex] = m_pool[index];
			m_pool[index] = currentBackElement;
		}
		
		m_usedEndIndex--;
	}

	bool IsEmpty() const
	{
		return m_pool.empty();
	}

	/// <summary>
	/// Gets the total amount of elements that can be stored. 
	/// Set by the value during pool construction
	/// </summary>
	/// <returns></returns>
	size_t GetMaxCapacity() const
	{
		return m_pool.capacity();
	}
	bool IsAtCapacity() const
	{
		return GetMaxCapacity() == m_pool.size();
	}
	/// <summary>
	/// Gets ONLY THE SIZE FOR USED OBJECTS
	/// This does not include objects that might have been added but have been marked 
	/// as unused
	/// </summary>
	/// <returns></returns>
	size_t GetUsedSize() const
	{
		if (m_usedEndIndex == -1) return 0;
		return m_usedEndIndex + 1;
	}
	/// <summary>
	/// Gets the total objects stored (both in use and those reserved if more added)
	/// </summary>
	/// <returns></returns>
	size_t GetObjectsStored() const
	{
		return m_pool.size();
	}

	/// <summary>
	/// Return true if there are any elements not being used that can be set
	/// </summary>
	/// <returns></returns>
	bool HasUnusedAvailable() const
	{
		if (m_usedEndIndex == -1) return m_pool.size() > 0;
		return m_usedEndIndex < m_pool.size() - 1;
	}

	/// <summary>
	/// Gets the index of the last usable element in the pool.
	/// All unused objects are stored at the end, so 
	/// total objects stored - unused objects -1 = this value
	/// </summary>
	/// <returns></returns>
	size_t GetLastUsableIndex() const
	{
		return m_usedEndIndex;
	}

	void ExecuteOnAvailable(const std::function<void(T& element, const size_t& index)>& func)
	{
		//If none of the elements are availabe we return
		//Assert(false, std::format("Unused index:{} max:{}", std::to_string(m_usedEndIndex), std::to_string(size_t(-1))));
		if (m_usedEndIndex == -1) return;

		size_t i = m_usedEndIndex;
		const size_t maxSize = -1;
		//Assert(false, std::format("Excecute with i:{} size:{}", std::to_string(i), std::to_string(m_pool.size())));
		while (i >= 0 && i!=maxSize)
		{
			//LogError(std::format("I is:{}", std::to_string(i)));
			func(GetAtUnsafe(i), i);
			i--;
		}
	}
};

