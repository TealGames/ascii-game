#pragma once
#include <utility>
#include <stdexcept>
#include <type_traits>
#include <cstring>

template <typename TObj, typename TReturn, typename...TArgs>
struct MemberFunction
{
	using FuncPtr = TReturn(TObj::*)(TArgs...);

	TObj* m_Obj;
	FuncPtr m_Func;

	MemberFunction() : m_Obj(nullptr), m_Func(nullptr) {}
	MemberFunction(TObj* objPtr, FuncPtr funcPtr) : m_Obj(objPtr), m_Func(funcPtr) {}

	explicit operator bool() const
	{
		return m_Obj != nullptr && m_Func != nullptr;
	}

	TReturn operator()(TArgs&&... args)
	{
		if (m_Obj == nullptr || m_Func == nullptr)
			throw std::runtime_error("Attempted to invoke MemberFunction on null object and/or function");
		
		return (m_Obj->*m_Func)(std::forward<TArgs>(args)...);
	}
};

template<typename TReturn, typename... TArgs>
struct ErasedMemberFunction
{
    using Invoker = TReturn(*)(void*, void*, TArgs&&...);

    void* m_Obj = nullptr;
    typename std::aligned_storage<sizeof(void*) * 2, alignof(void*)>::type m_FuncStorage{};
    Invoker m_Invoker = nullptr;

    explicit operator bool() const
    {
        return m_Obj != nullptr && m_Invoker != nullptr;
    }

    template<typename T>
    void Bind(T* obj, TReturn(T::* funcPtr)(TArgs...))
    {
        m_Obj = obj;

        // Copy the pointer-to-member-function into the storage
        static_assert(sizeof(funcPtr) <= sizeof(m_FuncStorage),
            "Func storage not large enough for this PMF");
        std::memcpy(&m_FuncStorage, &funcPtr, sizeof(funcPtr));

        // Create a trampoline to call the stored PMF
        m_Invoker = [](void* typeErasedObj, void* funcStorage, TArgs&&... args) -> TReturn
            {
                TReturn(T:: * realFunc)(TArgs...);
                std::memcpy(&realFunc, funcStorage, sizeof(realFunc));
                return (static_cast<T*>(typeErasedObj)->*realFunc)(std::forward<TArgs>(args)...);
            };
    }

    TReturn operator()(TArgs&&... args) const
    {
        if (m_Obj==nullptr || m_Invoker==nullptr)
            throw std::runtime_error("Attempted to invoke unbound MemberFunction");

        return m_Invoker(m_Obj, const_cast<void*>(reinterpret_cast<const void*>(&m_FuncStorage)),
            std::forward<TArgs>(args)...);
    }
};