#pragma once
#include <optional>
#include "CameraSettings.hpp"
#include "TextBuffer.hpp"

struct CameraData
{
	CameraSettings m_CameraSettings;
	std::optional<TextBuffer> m_LastFrameBuffer;

	CameraData();
	CameraData(const CameraSettings& cameraSettings);
};