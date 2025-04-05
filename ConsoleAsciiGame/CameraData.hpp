#pragma once
#include <optional>
#include "CameraSettings.hpp"
#include "ComponentData.hpp"
#include "TextBuffer.hpp"
#include "WorldPosition.hpp"
#include <vector>

class CameraData : public ComponentData
{
public:
	CameraSettings m_CameraSettings;
	TextBufferMixed m_LastFrameBuffer;

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