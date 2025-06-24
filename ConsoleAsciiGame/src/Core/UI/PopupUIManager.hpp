#pragma once
#include <cstdint>
#include <type_traits>
#include <limits>
#include "Utils/HelperFunctions.hpp"
#include "Core/Analyzation/Debug.hpp"
#include "Core/UI/PopupUI.hpp"
#include "Utils/Data/Event.hpp"
#include "Core/UI/UIRect.hpp"
#include "Core/UI/UIHierarchy.hpp"
#include "Utils/HelperMacros.hpp"

template<typename T>
concept IsPopupType = std::is_base_of_v<PopupUI, T>;

//using RenderPriority = std::uint8_t;
//constexpr RenderPriority HIGHEST_PRIORITY = std::numeric_limits<RenderPriority>::max();
//constexpr RenderPriority LOWEST_PRIORITY = std::numeric_limits<RenderPriority>::min();

struct PopupGUIInfo
{
	PopupUI* m_UI;

	PopupGUIInfo(PopupUI& gui);
	bool IsEnabled() const;

	void Enable();
	void Disable();
};

enum class PopupPositionFlags : std::uint8_t
{
	None				= 0,
	BelowRect			= 1<<0,
	CenteredXToRect		= 1<<1
};
FLAG_ENUM_OPERATORS(PopupPositionFlags)

class UIHierarchy;
class EntityData;
class PopupUIManager
{
private:
	UIHierarchy* m_hierarchy;
	EntityData* m_popupContainer;
	std::vector<PopupGUIInfo> m_popups;
public:
	/// <summary>
	/// Invokes when a popup is OPENED where string is the type after typeid()
	/// </summary>
	Event<void, std::string, const PopupUI*> m_OnPopupOpened;
	/// <summary>
	/// Invokes when a popup is CLOSED where string is the type after typeid()
	/// </summary>
	Event<void, std::string, const PopupUI*> m_OnPopupClosed;

private:
	template<typename T>
	requires IsPopupType<T>
	T* TryConvertPopup(PopupUI* gui)
	{
		try
		{
			return dynamic_cast<T*>(gui);
		}
		catch (const std::exception& e)
		{
			Assert(false, std::format("Tried to convert popup mutable of type:{} "
				"but could not convert found type:{}", Utils::GetTypeName<T>(),
				Utils::GetTypeName<decltype(*gui)>()));
		}
		return nullptr;
	}

	template<typename T>
	requires IsPopupType<T>
	PopupGUIInfo* TryGetPopupInfoMutable()
	{
		if (m_popups.empty()) return nullptr;

		for (auto& popup : m_popups)
		{
			if (popup.m_UI == nullptr) continue;
			if (typeid(*popup.m_UI).name() == typeid(T).name())
				return &popup;
		}
		return nullptr;
	}

	template<typename T>
	requires IsPopupType<T>
	T* TryOpenPopupAt(PopupGUIInfo& popupInfo, const ScreenPosition& pos)
	{
		return TryConvertPopup<T>(OpenPopupAtSimple(popupInfo, pos));
		/*try
		{
			return dynamic_cast<T*>(OpenPopupAtSimple(popupInfo, pos));
		}
		catch (const std::exception& e)
		{
			Assert(false, std::format("Tried to open popup mutable of type:{} at pos:{} "
				"but could not convert found type:{}", Utils::GetTypeName<T>(), pos.ToString(),
				Utils::GetTypeName<decltype(*(popupInfo.m_GUI))>()));
		}
		return nullptr;*/
	}
	template<typename T>
	requires IsPopupType<T>
	T* TryOpenPopupAt(PopupGUIInfo& popupInfo, const UIRect& rect, const PopupPositionFlags flags)
	{
		return TryConvertPopup<T>(OpenPopupAtSimple(popupInfo, rect, flags));
		/*try
		{
			return dynamic_cast<T*>(OpenPopupAtSimple(popupInfo, rect, flags));
		}
		catch (const std::exception& e)
		{
			Assert(false, std::format("Tried to open popup mutable of type:{} with rect:{} "
				"but could not convert found type:{}", Utils::GetTypeName<T>(), rect.ToString(),
				Utils::GetTypeName<decltype(*(popupInfo.m_GUI))>()));
		}
		return nullptr;*/
	}

	/// <summary>
	/// Precondition: popupinfo is NOT nullptr
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="popupInfo"></param>
	/// <param name="pos"></param>
	/// <returns></returns>
	PopupUI* OpenPopupAtSimple(PopupGUIInfo& popupInfo, const ScreenPosition& pos);
	PopupUI* OpenPopupAtSimple(PopupGUIInfo& popupInfo, const UIRect& rect, const PopupPositionFlags flags);

	/// <summary>
	/// Precondition: popupinfo is NOT nullptr
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="popupInfo"></param>
	/// <returns></returns>
	bool TryClosePopup(PopupGUIInfo& popupInfo);

public:
	PopupUIManager(UIHierarchy& hierarchy);
	~PopupUIManager();

	void AddPopup(PopupUI* poup);

	template<typename T> 
	requires IsPopupType<T>
	T* TryGetPopupMutable()
	{
		PopupGUIInfo* infoPtr = TryGetPopupInfoMutable<T>();
		if (infoPtr == nullptr) return nullptr;

		return TryConvertPopup<T>(infoPtr->m_UI);
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
	T* TryOpenPopupAt(const UIRect& rect, const PopupPositionFlags flags)
	{
		PopupGUIInfo* infoPtr = TryGetPopupInfoMutable<T>();
		if (infoPtr == nullptr) return nullptr;
	
		return TryOpenPopupAt<T>(*infoPtr, rect, flags);
	}

	/// <summary>
	/// Will toggle the popup and return TRUE if it is OPENED and FALSE if it is CLOSED
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="pos"></param>
	/// <param name="outPopup"></param>
	/// <returns></returns>
	template<typename T>
	requires IsPopupType<T>
	bool TogglePopupAt(const ScreenPosition& pos, T** outPopup=nullptr)
	{
		PopupGUIInfo* infoPtr = TryGetPopupInfoMutable<T>();
		if (infoPtr == nullptr)
		{
			Assert(false, std::format("Attempted to toggle popup at pos:{} with type:{} "
				"but no popup of that type could be found", pos.ToString(), Utils::FormatTypeName(typeid(T).name())));
			return false;
		}

		const bool isEnabled = infoPtr->IsEnabled();
		isEnabled? TryClosePopup(*infoPtr) : OpenPopupAtSimple(*infoPtr, pos);
		
		if (outPopup!=nullptr) *outPopup= TryConvertPopup<T>(infoPtr->m_UI);
		return !isEnabled;
	}

	template<typename T>
	requires IsPopupType<T>
	bool TryTogglePopupAt(const UIRect& rect, const PopupPositionFlags flags, T** outPopup = nullptr)
	{
		PopupGUIInfo* infoPtr = TryGetPopupInfoMutable<T>();
		if (infoPtr == nullptr)
		{
			Assert(false, std::format("Attempted to toggle popup at rect:{} with type:{} "
				"but no popup of that type could be found", rect.ToString(), Utils::FormatTypeName(typeid(T).name())));
			return false;
		}

		const bool isEnabled = infoPtr->IsEnabled();
		isEnabled ? TryClosePopup(*infoPtr) : OpenPopupAtSimple(*infoPtr, rect, flags);

		if (outPopup != nullptr) *outPopup = TryConvertPopup<T>(infoPtr->m_UI);
		return !isEnabled;
	}

	template<typename T>
	requires IsPopupType<T>
	bool TryClosePopup()
	{
		PopupGUIInfo* infoPtr = TryGetPopupInfoMutable<T>();
		if (infoPtr == nullptr) return false;

		return TryClosePopup(*infoPtr);
	}

	template<typename T>
	requires IsPopupType<T>
	bool IsPopupType(const std::string& str)
	{
		return str == Utils::FormatTypeName(typeid(T).name());
	}

	void CloseAllPopups();
};

