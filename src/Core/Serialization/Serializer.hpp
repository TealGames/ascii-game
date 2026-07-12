#pragma once
#include "Core/Serialization/JsonSerializers.hpp"

namespace Engine::Serialization
{
	class Serializer
	{
	private:
		ISerializerImpl* m_impl;
	public:

	private:
	public:
		Serializer(ISerializerImpl* impl);

		template<typename T>
		void AddProperty(const char* propertyName, const T& propertyValue)
		{
			if (typeid(*m_impl) == typeid(JsonSerializerImpl))
				dynamic_cast<JsonSerializerImpl*>(m_impl)->AddProperty(propertyName, propertyValue);
			else
			{
#if ENGINE_DEBUG
				LogError(std::format("Attempted to add property on a serializer but it has no valid impl actions"));
#endif
			}
		}
	};

	class Deserializer
	{
	private:
		IDeserializerImpl* m_impl;
	public:

	private:
	public:
		Deserializer(IDeserializerImpl* impl);

		template<typename T>
		void GetProperty(const char* propertyName, T* outPropertyValue)
		{
			if (typeid(*m_impl) == typeid(JsonDeserializerImpl))
				dynamic_cast<JsonDeserializerImpl*>(m_impl)->GetProperty(propertyName, outPropertyValue);
			else
			{
#if ENGINE_DEBUG
				LogError(std::format("Attempted to get property on a deserializer but it has no valid impl actions"));
#endif
			}
		}
	};
}