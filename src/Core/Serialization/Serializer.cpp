#include "Core/Serialization/Serializer.hpp"

namespace Engine::Serialization
{
	Serializer::Serializer(ISerializerImpl* impl) : m_impl(impl) {}

	Deserializer::Deserializer(IDeserializerImpl* impl) : m_impl(impl) {}
}