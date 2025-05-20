#pragma once
#include <vector>
#include <functional>
#include <limits>
#include <format>
#include <cstdint>

template <typename TReturn, typename...TArgs>
class Event
{
private:
	using ListenerType = std::function<TReturn(TArgs...)>;
	std::vector<ListenerType> m_listeners;
	std::uint8_t m_maxListenersAllowed;

public:
private:
	std::vector<ListenerType>::const_iterator TryGetIteratorForListener
	(const std::function<TReturn(TArgs...)> listener) const
	{
		int index = 0;
		typename std::vector<ListenerType>::const_iterator it = m_listeners.end();
		for (const ListenerType& existingListener : m_listeners)
		{
			if (existingListener.target_type() == listener.target_type())
			{
				it = m_listeners.begin() + index;
				break;
			}
		}
		return it;
	}

	std::vector<ListenerType>::const_iterator HasIteratorForListener
	(const std::function<TReturn(TArgs...)> listener) const
	{
		return TryGetIteratorForListener(listener) != m_listeners.end();
	}

	std::string GetSignatureStr() const
	{
		std::string result = std::format("{}(", typeid(TReturn).name());

		std::string arg_list = "";
		((arg_list += (arg_list.empty() ? "" : ", ") + std::string(typeid(TArgs).name())), ...);

		result += arg_list + ")";
		return result;
	}

public:
	Event(const std::uint8_t maxListeners = std::numeric_limits<std::uint8_t>::max())
		: m_listeners{}, m_maxListenersAllowed(maxListeners)
	{

	}
	~Event()
	{
		m_listeners.clear();
	}

	/// <summary>
	/// Returns true if it successfully invoked 1+ listeners
	/// </summary>
	/// <returns></returns>
	bool Invoke(TArgs... args)
	{
		if (m_listeners.empty()) return false;

		for (ListenerType& listener : m_listeners)
		{
			if (!listener)
			{
				throw std::invalid_argument(std::format("Tried to invoke an event with signature: "
					"'{}' but found invalid listener", GetSignatureStr()));
				return false;
			}

			listener(args...);
		}
		return true;
	}

	void AddListener(const std::function<TReturn(TArgs...)>& listener)
	{
		if (m_listeners.size() >= m_maxListenersAllowed)
		{
			throw std::invalid_argument(std::format("Tried to add listener to event with signature:'{}' but it "
				"has already reached its max listener limit: {}", GetSignatureStr(), m_maxListenersAllowed));
			return;
		}

		if (!listener)
		{
			throw std::invalid_argument(std::format("Tried to add listener to event with signature: "
				"'{}' but function was invalid", GetSignatureStr()));
			return;
		}

		m_listeners.emplace_back(listener);
	}

	bool HasListener(const std::function<TReturn(TArgs...)> listener) const
	{
		return HasIteratorForListener(listener);
	}

	bool TryRemoveListener(const std::function<TReturn(TArgs...)> listener)
	{
		auto foundIt = TryGetIteratorForListener(listener);
		if (foundIt == m_listeners.end()) return false;
		int index = 0;

		m_listeners.erase(foundIt);
		return true;
	}
	void RemoveAllListeners()
	{
		m_listeners.clear();
	}

	const std::vector<ListenerType>& GetListeners() const
	{
		return m_listeners;
	}

	size_t GetListenerCount() const
	{
		return m_listeners.size();
	}
	bool HasListeners() const
	{
		return GetListenerCount() > 0;
	}
};