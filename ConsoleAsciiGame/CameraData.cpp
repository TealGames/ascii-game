#include "pch.hpp"

#include "CameraData.hpp"
#include "CameraSettings.hpp"
#include "TextBuffer.hpp"

CameraData::CameraData() :
	m_CameraSettings(), m_LastFrameBuffer(std::nullopt){}

CameraData::CameraData(const CameraSettings& cameraSettings) :
	m_CameraSettings(cameraSettings), m_LastFrameBuffer(std::nullopt) {}