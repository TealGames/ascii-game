#pragma once

/// <summary>
/// Represents an object that can be changed into one form and back to another
/// where TargetType is the default representation and TransportType is the format for serialization
/// </summary>
/// <typeparam name="TargetType"></typeparam>
/// <typeparam name="TransformType"></typeparam>
template<typename TargetType,typename TransportType>
class ISerializable
{
private:
public:

private:
public:
	ISerializable() = default;
	~ISerializable() = default;

	virtual TransportType Serialize(const TargetType& target) const = 0;
	virtual TargetType Deserialize(const TransportType& transport) const = 0;
};

