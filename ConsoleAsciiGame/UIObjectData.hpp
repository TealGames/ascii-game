#pragma once
#include "Component.hpp"
#include "NormalizedPosition.hpp"

class UIObjectData : public Component
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

	//std::vector<std::string> GetDependencyFlags() const override;
	void InitFields() override;

	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
};

