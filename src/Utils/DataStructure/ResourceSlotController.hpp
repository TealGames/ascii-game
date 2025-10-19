#include <array>
#include <deque>
#include "Utils/Debug.hpp"
#include "Utils/ToStringFunctions.hpp"

using SlotIndex = int;
using USlotIndex = std::uint16_t;
inline constexpr SlotIndex INVALID_SLOT_INDEX = -1;

template<typename T, USlotIndex N>
class ResourceSlotController
{
private:
	std::array<T, N> m_slots;
	std::deque<SlotIndex> m_indices;
public:

private:
public:
	ResourceSlotController() : m_slots{}, m_indices() 
	{
		Clear();
	}

	SlotIndex AddUnsafe(T&& resource)
	{
		const SlotIndex index = m_indices.front();
		m_slots[index] = std::forward<T>(resource);
		m_indices.pop_front();

		return index;
	}
	SlotIndex TryAdd(T&& resource)
	{
		if (IsFull())
		{
			return INVALID_SLOT_INDEX;
		}

		return AddUnsafe(std::forward<T>(resource));
	}

	void RemoveAtUnsafe(const USlotIndex& index)
	{
		m_indices.push_front(index);
	}
	void RemoveAt(const USlotIndex& index)
	{
		if (index >= N)
		{
			LogError("Tried to get resource of slot controller at index: {} but it is out of range: [0,{})",
				std::to_string(index), std::to_string(N));
			throw std::invalid_argument("Invalid resource slot controller index");
		}
		RemoveAtUnsafe(index);
	}
	/// <summary>
	/// Assumes there is already a resource at this slot 
	/// (and is there NOT an index that needs to be popped)
	/// </summary>
	/// <param name="index"></param>
	/// <param name="resource"></param>
	/// <returns></returns>
	T ReplaceAt(const USlotIndex& index, T&& resource)
	{
		T oldResource = m_slots[index];
		m_slots[index] = std::forward<T>(resource);
		return oldResource;
	}

	T& GetAt(const USlotIndex& index)
	{
		if (index >= N)
		{
			LogError("Tried to get resource of slot controller at index: {} but it is out of range: [0,{})", 
				std::to_string(index), std::to_string(N));
			throw std::invalid_argument("Invalid resource slot controller index");
		}
		return m_slots[index];
	}
	T& operator[](const USlotIndex& index)
	{
		return m_slots[index];
	}
	const T& operator[](const USlotIndex& index) const
	{
		return m_slots[index];
	}

	void Clear()
	{
		while (!m_indices.empty())
			m_indices.pop_front();

		for (SlotIndex i = 0; i < N; i++)
		{
			m_indices.push_back(i);
		}
	}

	size_t GetSize() const { return N; }
	size_t GetEmptySlotCount() const { return m_indices.size(); }

	bool IsEmpty() const { return m_indices.size() == N; }
	bool IsFull() const { return m_indices.size() == 0; }

	auto begin() { return m_slots.begin(); }
	auto begin() const { return m_slots.begin(); }

	auto end() { return m_slots.end(); }
	auto end() const { return m_slots.end(); }
};