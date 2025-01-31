#include "pch.hpp"

#include "CameraData.hpp"
#include "CameraSettings.hpp"
#include "TextBuffer.hpp"
#include "ComponentData.hpp"

CameraData::CameraData() :
	ComponentData(), m_CameraSettings(), m_LastFrameBuffer(){}

CameraData::CameraData(const CameraSettings& cameraSettings) :
	ComponentData(), m_CameraSettings(cameraSettings), m_LastFrameBuffer() {}