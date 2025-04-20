#pragma once
#include <unordered_map>
#include <vector>
#include <optional>
#include "HelperFunctions.hpp"
#include "Debug.hpp"

/// <summary>
/// A map that retains the insertion order. It is implemented by having a hashtable that stores key values normally so
/// we still have the fast insertion and quick key lookup. However, there is a vector that stores the key order so we can
/// have fast retrieval for ALL values (something that is difficult with fragmented values in a map) and so we can 
/// maintain the same order as the value pairs were addded
/// </summary>
/// <typeparam name="KType"></typeparam>
/// <typeparam name="VType"></typeparam>
template<typename KType, typename VType>
class PreservedMap
{
public:
	using MapType = std::unordered_map<KType, VType>;
	using KeyOrderType = std::vector<KType>;

	class Iterator
	{
	private:
		MapType* m_mapPtr;
		KeyOrderType* m_keyOrderPtr;
		size_t m_currentIndex;
	public:

	private:
		size_t SetIndex(const size_t& newIndex)
		{
			//Note: we allow last index (not size -1) since that it reserved for invalid iterators like .end()
			m_currentIndex = std::clamp(newIndex, size_t(0), m_keyOrderPtr->size());
			return m_currentIndex;
		}

	public:
		Iterator(MapType& map, KeyOrderType& keyOrder, const size_t& currentIndex)
			: m_mapPtr(&map), m_keyOrderPtr(&keyOrder), m_currentIndex(0)
			{ SetIndex(currentIndex); }

		//Note: try to avoid the iterator constructors because it requires a linear search of key order to find iterator that matches map key
		//(distance is O(1) since it is pointer substraction because of contiguous values)
		Iterator(MapType& map, KeyOrderType& keyOrder, const MapType::const_iterator& it)
			: m_mapPtr(&map), m_keyOrderPtr(&keyOrder), m_currentIndex(0) 
			{ SetIndex(it==m_mapPtr->cend()? -1 : std::distance(m_keyOrderPtr->cbegin(), m_keyOrderPtr.find(it->first))); }

		//Note: try to avoid the iterator constructors because it requires a linear search of key order to find iterator that matches map key
		//(distance is O(1) since it is pointer substraction because of contiguous values)
		Iterator(MapType& map, KeyOrderType& keyOrder, const MapType::iterator& it)
			: m_mapPtr(&map), m_keyOrderPtr(&keyOrder), m_currentIndex(0) 
			{ SetIndex(it == m_mapPtr->end() ? -1 : std::distance(m_keyOrderPtr->begin(), m_keyOrderPtr.find(it->first))); }

		KType& GetKeyMutable()
		{
			if (m_currentIndex == m_keyOrderPtr->size())
				throw std::invalid_argument(std::format("Attempted to get key MUTABLE from invalid iterator at index:{} (valid range:[0,{}]", 
					std::to_string(m_currentIndex), std::to_string(m_keyOrderPtr->size()-1)));

			return m_keyOrderPtr->at(m_currentIndex);
		}
		const KType& GetKey() const
		{
			if (m_currentIndex == m_keyOrderPtr->size())
				throw std::invalid_argument(std::format("Attempted to get key from invalid iterator at index:{} (valid range:[0,{}]",
					std::to_string(m_currentIndex), std::to_string(m_keyOrderPtr->size() - 1)));

			return m_keyOrderPtr->at(m_currentIndex);
		}

		VType& GetValueMutable()
		{
			const KType& keyValue = GetKey();
			return m_mapPtr->at(keyValue);
		}
		const VType& GetValue() const
		{
			const KType& keyValue = GetKey();
			return m_mapPtr->at(keyValue);
		}

		std::string ToString() const
		{
			return Utils::ToStringPair<KType, VType>(GetKey(), GetValue());
		}

		//Note: std iterators normally are pair refs due to how they are stored internally
		//however since we have different structure, we have to make each key value a pointer
		//to prevent copying
		const std::pair<const KType*, const VType*> operator*() const
		{
			return {&GetKey(), &GetValue()};
		}

		bool operator==(const Iterator& other) const
		{
			return m_currentIndex == other.m_currentIndex 
				&& m_mapPtr == other.m_mapPtr;
		}
		bool operator!=(const Iterator& other) const
		{
			return !(*this == other);
		}

		Iterator& operator++()
		{
			SetIndex(m_currentIndex + 1);
			return *this;
		}
		Iterator& operator+(const size_t& delta)
		{
			SetIndex(m_currentIndex + delta);
			return *this;
		}
		Iterator& operator--()
		{
			SetIndex(m_currentIndex - 1);
			return *this;
		}
		Iterator& operator-(const size_t& delta)
		{
			SetIndex(m_currentIndex + delta);
			return *this;
		}
	};

	class ConstIterator
	{
	private:
		const MapType* m_mapPtr;
		const KeyOrderType* m_keyOrderPtr;
		size_t m_currentIndex;
	public:

	private:
		size_t SetIndex(const size_t& newIndex)
		{
			//Note: we allow last index (not size -1) since that it reserved for invalid iterators like .end()
			m_currentIndex = std::clamp(newIndex, size_t(0), m_keyOrderPtr->size());
			return m_currentIndex;
		}

	public:
		ConstIterator(const MapType& map, const KeyOrderType& keyOrder, const size_t& currentIndex)
			: m_mapPtr(&map), m_keyOrderPtr(&keyOrder), m_currentIndex(0) 
			{ SetIndex(currentIndex); }

		//Note: try to avoid the iterator constructors because it requires a linear search of key order to find iterator that matches map key
		//(distance is O(1) since it is pointer substraction because of contiguous values)
		ConstIterator(const MapType& map, const KeyOrderType& keyOrder, const MapType::const_iterator& it)
			: m_mapPtr(&map), m_keyOrderPtr(&keyOrder), m_currentIndex(0) 
			{ SetIndex(it == m_mapPtr->cend() ? -1 : std::distance(m_keyOrderPtr->cbegin(), m_keyOrderPtr.find(it->first))); }

		const KType& GetKey() const
		{
			if (m_currentIndex == m_keyOrderPtr->size())
				throw std::invalid_argument(std::format("Attempted to get key from invalid const iterator at index:{} (valid range:[0,{}]",
					std::to_string(m_currentIndex), std::to_string(m_keyOrderPtr->size() - 1)));

			return m_keyOrderPtr->at(m_currentIndex);
		}
		const VType& GetValue() const
		{
			const KType& keyValue = GetKey();
			return m_mapPtr->at(keyValue);
		}

		std::string ToString() const
		{
			return Utils::ToStringPair<KType, VType>(GetKey(), GetValue());
		}

		const std::pair<const KType*, const VType*> operator*() const
		{
			return { &GetKey(), &GetValue() };
		}

		bool operator==(const ConstIterator& other) const
		{
			return m_currentIndex == other.m_currentIndex
				&& m_mapPtr == other.m_mapPtr;
		}
		bool operator!=(const ConstIterator& other) const
		{
			return !(*this == other);
		}

		ConstIterator& operator++()
		{
			SetIndex(m_currentIndex + 1);
			return *this;
		}
		ConstIterator& operator+(const size_t& delta)
		{
			SetIndex(m_currentIndex + delta);
			return *this;
		}
		ConstIterator& operator--()
		{
			SetIndex(m_currentIndex - 1);
			return *this;
		}
		ConstIterator& operator-(const size_t& delta)
		{
			SetIndex(m_currentIndex + delta);
			return *this;
		}
	};

private:
	MapType m_map;
	KeyOrderType m_keysOrdered;

private:
	size_t GetEndIteratorIndex() const { return m_keysOrdered.size(); }

public:
	PreservedMap() : m_map(), m_keysOrdered() {}

	std::pair<Iterator, bool> Insert(const KType& key, const VType& value)
	{
		if (!Assert(this, !HasKey(key), std::format("Tried to insert key value pair:{} but key already exists", 
			Utils::ToStringPair<KType, VType>(key, value))))
			return { EndMutable(), false };

		const std::pair<MapType::iterator, bool> mapResult = m_map.emplace(key, value);
		m_keysOrdered.push_back(key);

		if (typeid(KType) == typeid(std::string) && typeid(value)== typeid(std::vector<std::string>)) 
			LogError(std::format("Added pair: {} value:{} iterator result:{} full:{}", key, Utils::ToStringIterable<VType, std::string>(value),
				mapResult.first->first, ToString(true)));

		if (mapResult.first == m_map.end()) return {EndMutable(), mapResult.second };
		else return { Iterator(m_map, m_keysOrdered, m_keysOrdered.size()-1), mapResult.second};
	}

	size_t Erase(const KType& key)
	{
		size_t elementsRemoved = m_map.erase(key);

		//TODO: is there a way to optimize this?
		for (size_t i = 0; i < m_keysOrdered.size(); i++)
		{
			if (m_keysOrdered[i] != key) continue;
			
			m_keysOrdered.erase(m_keysOrdered.begin() + i);
			break;
		}

		return elementsRemoved;
	}

	/// <summary>
	/// Will find the key in the map and return a constant iterator to that key value pair
	/// Note: max of O(n) time complexity due to requiring an index value for the iterator
	/// which can not be found without going through every value in the key order list
	/// </summary>
	/// <param name="key"></param>
	/// <returns></returns>
	ConstIterator Find(const KType& key) const
	{
		if (!HasKey(key)) return End();

		for (size_t i = 0; i < m_keysOrdered.size(); i++)
		{
			if (m_keysOrdered[i] == key)
				return ConstIterator(m_map, m_keysOrdered, i);
		}

		Assert(false, std::format("PreservedMap tried to find key: {} when HasKey returned "
			"TRUE but could not be found", Utils::TryToString<KType>(key).value_or("")));
		return End();
	}
	/// <summary>
	/// Will find the key in the map and return a mutable iterator to that key value pair
	/// Note: max of O(n) time complexity due to requiring an index value for the iterator
	/// which can not be found without going through every value in the key order list
	/// </summary>
	/// <param name="key"></param>
	/// <returns></returns>
	Iterator FindMutable(const KType& key)
	{
		if (!HasKey(key)) return End();

		for (size_t i = 0; i < m_keysOrdered.size(); i++)
		{
			if (m_keysOrdered[i] == key)
				return ConstIterator(m_map, m_keysOrdered, i);
		}

		Assert(false, std::format("PreservedMap tried to find key MUTABLE: {} when HasKey returned "
			"TRUE but could not be found", Utils::TryToString<KType>(key)));
		return End();
	}

	const VType& At(const KType& key) const
	{
		return m_map.at(key);
	}
	const VType& At(const size_t& index) const
	{
		return At(m_keysOrdered.at(index));
	}
	VType& AtMutable(const KType& key)
	{
		return m_map.at(key);
	}
	VType& At(const size_t& index)
	{
		return At(m_keysOrdered.at(index));
	}

	bool HasKey(const KType& key) const
	{
		return m_map.find(key) != m_map.end();
	}

	std::vector<ConstIterator> RetrieveAll() const
	{
		std::vector<ConstIterator> iteratorsOrdered = {};
		if (m_keysOrdered.empty()) return iteratorsOrdered;

		for (size_t i=0; i<m_keysOrdered.size(); i++)
		{
			iteratorsOrdered.push_back(ConstIterator(m_map, m_keysOrdered, i));
			/*if (typeid(KType) == typeid(std::string) && typeid(VType) == typeid(std::vector<std::string>)) 
				LogError(std::format("Retrieve all key:{} value:{}", m_keysOrdered[i], Utils::ToStringIterable<VType, std::string>(m_map.at(m_keysOrdered[i]))));*/
		}
		return iteratorsOrdered;
	}

	size_t Size() const
	{
		return m_keysOrdered.size();
	}

	bool IsEmpty() const
	{
		return m_keysOrdered.empty();
	}

	Iterator BeginMutable()
	{
		return Iterator(m_map, m_keysOrdered, 0);
	}
	ConstIterator Begin() const
	{
		return ConstIterator(m_map, m_keysOrdered, 0);
	}
	Iterator begin() { return BeginMutable(); }
	ConstIterator begin() const { return Begin(); }

	Iterator EndMutable()
	{
		return Iterator(m_map, m_keysOrdered, GetEndIteratorIndex());
	}
	ConstIterator End() const
	{
		return ConstIterator(m_map, m_keysOrdered, GetEndIteratorIndex());
	}
	Iterator end() { return EndMutable(); }
	ConstIterator end() const { return End(); }

	std::string ToString(const bool displayMapAndKeyOrder = false) const
	{
		std::string mapAndKeyOrderStr = "";
		if (displayMapAndKeyOrder)
		{
			mapAndKeyOrderStr += std::format("[KeyOrder:{}]", Utils::ToStringIterable<KeyOrderType, KType>(m_keysOrdered));
			mapAndKeyOrderStr += std::format("[Map:{}]", Utils::ToStringIterable<KType, VType>(m_map));
		}

		std::vector<std::string> elementsStr = {};
		for (const auto& pair : RetrieveAll())
		{
			if (typeid(KType) == typeid(std::string) && typeid(VType) == typeid(std::vector<std::string>))
				elementsStr.push_back(std::format("[K:{} V:{}]", pair.GetKey(), Utils::ToStringIterable<VType, std::string>(pair.GetValue())));
			else elementsStr.push_back(Utils::ToStringPair<KType, VType>(pair.GetKey(), pair.GetValue()));
		}
		return std::format("[{}elements:{}]", displayMapAndKeyOrder? mapAndKeyOrderStr : "", 
			Utils::ToStringIterable<std::vector<std::string>, std::string>(elementsStr));
	}
};

