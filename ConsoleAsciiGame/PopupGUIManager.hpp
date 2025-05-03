#pragma once
#include <map>
#include <type_traits>
#include <limits>
#include "HelperFunctions.hpp"
#include "Debug.hpp"
#include "PopupGUI.hpp"
#include "Event.hpp"
#include "GUIRect.hpp"

template<typename T>
concept IsPopupType = std::is_base_of_v<PopupGUI, T>;

using RenderPriority = std::uint8_t;
constexpr RenderPriority HIGHEST_PRIORITY = std::numeric_limits<RenderPriority>::max();
constexpr RenderPriority LOWEST_PRIORITY = std::numeric_limits<RenderPriority>::min();

struct PopupGUIInfo
{
	static const ScreenPosition INVALID_RENDER_POS;

	PopupGUI* m_GUI;
	RenderInfo m_RenderInfo;

	PopupGUIInfo(PopupGUI& gui, const RenderInfo& info);
	bool IsEnabled() const;
};

class PopupGUIManager
{
private:

	std::map<RenderPriority, PopupGUIInfo, std::greater<RenderPriority>> m_popups;
public:
	Event<void, const PopupGUI*> m_OnPopupOpened;
	Event<void, const PopupGUI*> m_OnPopupClosed;

private:
	template<typename T>
	requires IsPopupType<T>
	PopupGUIInfo* TryGetPopupInfoMutable()
	{
		if (m_popups.empty()) return nullptr;

		for (auto& popup : m_popups)
		{
			if (popup.second.m_GUI == nullptr) continue;
			if (typeid(*popup.second.m_GUI).name() == typeid(T).name())
				return &popup.second;
		}
		return nullptr;
	}

	/// <summary>
	/// Precondition: popupinfo is NOT nullptr
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="popupInfo"></param>
	/// <param name="pos"></param>
	/// <returns></returns>
	template<typename T>
	requires IsPopupType<T>
	T* TryOpenPopupAt(PopupGUIInfo& popupInfo, const ScreenPosition& pos)
	{
		try
		{
			return dynamic_cast<T*>(OpenPopupAtSimple(popupInfo, pos));
		}
		catch (const std::exception& e)
		{
			Assert(false, std::format("Tried to open popup mutable of type:{} at pos:{} "
				"but could not convert found type:{}", Utils::GetTypeName<T>(), pos.ToString(),
				Utils::GetTypeName<decltype(*(popupInfo.m_GUI))>()));
		}
		return nullptr;
	}

	/// <summary>
	/// Precondition: popupinfo is NOT nullptr
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="popupInfo"></param>
	/// <param name="pos"></param>
	/// <returns></returns>
	PopupGUI* OpenPopupAtSimple(PopupGUIInfo& popupInfo, const ScreenPosition& pos);

	/// <summary>
	/// Precondition: popupinfo is NOT nullptr
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="popupInfo"></param>
	/// <returns></returns>
	bool TryClosePopup(PopupGUIInfo& popupInfo);

public:
	PopupGUIManager();
	~PopupGUIManager();

	void AddPopup(PopupGUI* poup, const ScreenPosition& targetSize, const RenderPriority priority);

	template<typename T> 
	requires IsPopupType<T>
	T* TryGetPopupMutable()
	{
		PopupGUIInfo* infoPtr = TryGetPopupInfoMutable<T>();
		if (infoPtr == nullptr) return nullptr;

		try
		{
			return dynamic_cast<T*>(infoPtr->m_GUI);
		}
		catch (const std::exception& e)
		{
			Assert(false, std::format("Tried to get popup mutable of type:{} "
				"but could not convert found type:{}", Utils::GetTypeName<T>(), 
				Utils::GetTypeName<decltype(*infoPtr->m_GUI)>()));
		}
		return nullptr;
	}

	template<typename T>
	requires IsPopupType<T>
	T* TryOpenPopupAt(const ScreenPosition& pos)
	{
		PopupGUIInfo* infoPtr = TryGetPopupInfoMutable<T>();
		if (infoPtr == nullptr) return nullptr;

		return TryOpenPopupAt<T>(*infoPtr, pos);
	}

	template<typename T>
	requires IsPopupType<T>
	bool TryTogglePopupAt(const ScreenPosition& pos)
	{
		PopupGUIInfo* infoPtr = TryGetPopupInfoMutable<T>();
		if (infoPtr == nullptr) return false;

		//Note: for toggle, we do not care about returning value from popup open, so we can skip dynamic cast
		return infoPtr->IsEnabled() ? TryClosePopup(*infoPtr) : OpenPopupAtSimple(*infoPtr, pos) != nullptr;
	}

	template<typename T>
	requires IsPopupType<T>
	bool TryTogglePopupBelowRect(const GUIRect& rect)
	{
		return TryTogglePopupAt<T>(rect.GetTopleftPos() + ScreenPosition(0, rect.GetSize().m_Y));
	}

	template<typename T>
	requires IsPopupType<T>
	bool TryClosePopup()
	{
		PopupGUIInfo* infoPtr = TryGetPopupInfoMutable<T>();
		if (infoPtr == nullptr) return false;

		return TryClosePopup(infoPtr);
	}

	void CloseAllPopups();

	void RenderPopups();
	void UpdatePopups(const float deltaTime);
};

