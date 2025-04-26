#pragma once
#include <optional>
#include "CameraSettings.hpp"
#include "ComponentData.hpp"
#include "WorldPosition.hpp"
#include "TextBuffer.hpp"
#include <vector>

class CameraData : public ComponentData
{
public:
	CameraSettings m_CameraSettings;
	std::vector<TextBufferCharPosition> m_LastFrameBuffer;

public:
	CameraData();
	CameraData(const Json& json);
	CameraData(const CameraSettings& cameraSettings);

	std::vector<std::string> GetDependencyFlags() const override;
	void InitFields() override;

	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
};