#pragma once
#include <optional>
#include "CameraSettings.hpp"
#include "ComponentData.hpp"
#include "TextBuffer.hpp"

class CameraData : public ComponentData
{
public:
	CameraSettings m_CameraSettings;
	std::optional<TextBuffer> m_LastFrameBuffer;

public:
	CameraData();
	CameraData(const CameraSettings& cameraSettings);
};