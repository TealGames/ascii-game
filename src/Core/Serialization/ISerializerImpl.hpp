#pragma once

namespace Engine::Serialization
{
	class ISerializerImpl
	{
	private:
	public:
		virtual ~ISerializerImpl() = default;
	};

	class IDeserializerImpl
	{
	private:
	public:
		virtual ~IDeserializerImpl() = default;
	};
}