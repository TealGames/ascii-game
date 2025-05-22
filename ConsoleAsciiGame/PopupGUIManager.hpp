#pragma once
#include <cstdint>
#include <map>
#include <type_traits>
#include <limits>
#include "HelperFunctions.hpp"
#include "Debug.hpp"
#include "PopupGUI.hpp"
#include "Event.hpp"
#include "GUIRect.hpp"
#include "ContainerGUI.hpp"
#include "GUIHierarchy.hpp"
#include "HelperMacros.hpp"

template<typename T>
concept IsPopupType = std::is_base_of_v<PopupGUI, T>;

using RenderPriority = std::uint8_t;
constexpr RenderPriority HIGHEST_PRIORITY = std::numeric_limits<RenderPriority>::max();
constexpr RenderPriority LOWEST_PRIORITY = std::numeric_limits<RenderPriority>::min();

struct PopupGUIInfo
{
	static const size_t INVALID_INDEX;

	PopupGUI* m_GUI;
	size_t m_ChildIndex;

	PopupGUIInfo(PopupGUI& gui);
	bool IsEnabled() const;
};

enum class PopupPositionFlags : std::uint8_t
{
	None				= 0,
	BelowRect			= 1<<0,
	CenteredXToRect		= 1<<1
};
FLAG_ENUM_OPERATORS(PopupPositionFlags)

class GUIHierarchy;
class PopupGUIManager
{
private:
	GUIHierarchy* m_hierarchy;
	ContainerGUI m_popupContainer;
	std::map<RenderPriority, PopupGUIInfo, std::greater<RenderPriority>> m_popups;
public:
	/// <summary>
	/// Invokes when a popup is OPENED where string is the type after typeid()
	/// </summary>
	Event<void, std::string, const PopupGUI*> m_OnPopupOpened;
	/// <summary>
	/// Invokes when a popup is CLOSED where string is the type after typeid()
	/// </summary>
	Event<void, std::string, const PopupGUI*> m_OnPopupClosed;

private:
	template<typename T>
	requires IsPopupType<T>
	T* TryConvertPopup(PopupGUI* gui)
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
			if (popup.second.m_GUI == nullptr) continue;
			if (typeid(*popup.second.m_GUI).name() == typeid(T).name())
				return &popup.second;
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
	T* TryOpenPopupAt(PopupGUIInfo& popupInfo, const GUIRect& rect, const PopupPositionFlags flags)
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
	PopupGUI* OpenPopupAtSimple(PopupGUIInfo& popupInfo, const ScreenPosition& pos);
	PopupGUI* OpenPopupAtSimple(PopupGUIInfo& popupInfo, const GUIRect& rect, const PopupPositionFlags flags);

	/// <summary>
	/// Precondition: popupinfo is NOT nullptr
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="popupInfo"></param>
	/// <returns></returns>
	bool TryClosePopup(PopupGUIInfo& popupInfo);

public:
	PopupGUIManager(GUIHierarchy& hierarchy);
	~PopupGUIManager();

	void AddPopup(PopupGUI* poup, const ScreenPosition& targetSize, const RenderPriority priority);

	template<typename T> 
	requires IsPopupType<T>
	T* TryGetPopupMutable()
	{
		PopupGUIInfo* infoPtr = TryGetPopupInfoMutable<T>();
		if (infoPtr == nullptr) return nullptr;

		return TryConvertPopup<T>(infoPtr->m_GUI);
		/*try
		{
			return dynamic_cast<T*>(infoPtr->m_GUI);
		}
		catch (const std::exception& e)
		{
			Assert(false, std::format("Tried to get popup mutable of type:{} "
				"but could not convert found type:{}", Utils::GetTypeName<T>(), 
				Utils::GetTypeName<decltype(*infoPtr->m_GUI)>()));
		}
		return nullptr;*/
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
	T* TryOpenPopupAt(const GUIRect& rect, const PopupPositionFlags flags)
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
		
		if (outPopup!=nullptr) *outPopup= TryConvertPopup<T>(infoPtr->m_GUI);
		return !isEnabled;
	}

	template<typename T>
	requires IsPopupType<T>
	bool TryTogglePopupAt(const GUIRect& rect, const PopupPositionFlags flags, T** outPopup = nullptr)
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

		if (outPopup != nullptr) *outPopup = TryConvertPopup<T>(infoPtr->m_GUI);
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

	//void RenderPopups();
	void UpdatePopups(const float deltaTime);
};

