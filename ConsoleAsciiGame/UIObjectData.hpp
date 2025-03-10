#pragma once
#include "ComponentData.hpp"
#include "NormalizedPosition.hpp"
#include "IJsonSerializable.hpp"

class UIObjectData : public ComponentData, public IJsonSerializable<UIObjectData>
{
private:
	/// <summary>
	/// The normalized pos of the ui Object where [0, 0] is the bottom left
	/// and the top right is [1, 1]
	/// </summary>
	NormalizedPosition m_normalizedPos;
public:

private:
public:
	UIObjectData();
	UIObjectData(const Json& json);
	UIObjectData(const NormalizedPosition& normalizedPos);

	const NormalizedPosition& GetNormalizedPos() const;

	void InitFields() override;

	std::string ToString() const override;

	UIObjectData& Deserialize(const Json& json) override;
	Json Serialize(const UIObjectData& component) override;
};

