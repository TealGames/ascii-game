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
	CameraData(const CameraSettings& cameraSettings);

	void InitFields() override;
};