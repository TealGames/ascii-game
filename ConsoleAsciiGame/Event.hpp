#pragma once
#include <vector>
#include <functional>
#include <limits>
#include "Debug.hpp"

template <typename TReturn, typename...TArgs>
class Event
{
private:
	using ListenerType = std::function<TReturn(TArgs...)>;
	std::vector<ListenerType> _listeners;

public:
	const int MaxListenersAllowed;

private:
	std::vector<ListenerType>::const_iterator TryGetIteratorForListener
	(const std::function<TReturn(TArgs...)> listener) const
	{
		int index = 0;
		typename std::vector<ListenerType>::const_iterator it = _listeners.end();
		for (const ListenerType& existingListener : _listeners)
		{
			if (existingListener.target_type() == listener.target_type())
			{
				it = _listeners.begin() + index;
				break;
			}
		}
		return it;
	}

	std::vector<ListenerType>::const_iterator HasIteratorForListener
	(const std::function<TReturn(TArgs...)> listener) const
	{
		return TryGetIteratorForListener(listener) != _listeners.end();
	}

public:
	Event(const int maxListeners = std::numeric_limits<int>::max())
		: _listeners{}, MaxListenersAllowed(maxListeners)
	{

	}

	/// <summary>
	/// Returns true if it successfully invoked 1+ listeners
	/// </summary>
	/// <returns></returns>
	bool Invoke(const TArgs&... args)
	{
		if (_listeners.empty()) return false;

		for (const ListenerType& listener : _listeners)
		{
			listener(args...);
		}
		return true;
	}

	void AddListener(const std::function<TReturn(TArgs...)>& listener)
	{
		if (_listeners.size() >= MaxListenersAllowed)
		{
			std::string err = std::format("Tried to add listener to event but it "
				"has already reached its max listener limit: {}", MaxListenersAllowed);
			LogError(this, err);
			return;
		}
		_listeners.push_back(listener);
	}

	bool HasListener(const std::function<TReturn(TArgs...)> listener) const
	{
		return HasIteratorForListener(listener);
	}

	bool TryRemoveListener(const std::function<TReturn(TArgs...)> listener)
	{
		auto foundIt = TryGetIteratorForListener(listener);
		if (foundIt == _listeners.end()) return false;
		int index = 0;

		_listeners.erase(foundIt);
		return true;
	}

	const std::vector<const ListenerType> GetListeners() const
	{
		std::vector<const ListenerType> listeners;
		for (const ListenerType& listener : _listeners)
		{
			listeners.push_back(listener);
		}
		return listeners;
	}
};