#pragma once
#include <optional>
#include "CameraSettings.hpp"
#include "ComponentData.hpp"
#include "TextBuffer.hpp"
#include "WorldPosition.hpp"
#include <vector>
#include "IJsonSerializable.hpp"

class CameraData : public ComponentData, public IJsonSerializable<CameraData>
{
public:
	CameraSettings m_CameraSettings;
	TextBufferMixed m_LastFrameBuffer;

public:
	CameraData();
	CameraData(const Json& json);
	CameraData(const CameraSettings& cameraSettings);

	void InitFields() override;

	std::string ToString() const override;

	CameraData& Deserialize(const Json& json) override;
	Json Serialize(const CameraData& component) override;
};