#include "pch.hpp"
#include "Core/Camera/CameraController.hpp"
#include "Core/Analyzation/Debug.hpp"

CameraController::CameraController() : m_priorityCameras() {}

bool CameraController::Validate()
{
	return Assert(this, !m_priorityCameras.empty(), 
		std::format("Validated camera controller but there are no cameras set which is not allowed"));
}

bool CameraController::TryRegisterCamera(CameraData& camera, const PriorityType& priority)
{
	//We make sure it is disabled by default so it does not attempt to render
	//until this controller decides what camera should be rendered
	camera.m_IsEnabled = false;

	//TODO: we should probavbly check to make sure we do not add duplicates
	m_priorityCameras.emplace(priority, &camera);
	return true;
}

void CameraController::UpdateActiveCamera()
{
	size_t currentIndex = 0;
	for (auto& priorityCamera : m_priorityCameras)
	{
		//Since cameras are ordered ascending, highest priority is last, and is the one enabled
		priorityCamera.second->m_IsEnabled = (currentIndex == m_priorityCameras.size() - 1);
		currentIndex++;
	}
}

CameraData& CameraController::GetActiveCameraMutable()
{
	if (!Assert(this, !m_priorityCameras.empty(), std::format("Tried to get active camera MUTABLE from "
		"camera controller, but there are no cameras registered!")))
		throw std::invalid_argument("Invalid camera state");

	return *(std::prev(m_priorityCameras.end())->second);
}

const CameraData& CameraController::GetActiveCamera() const
{
	if (!Assert(this, !m_priorityCameras.empty(), std::format("Tried to get active camera from "
		"camera controller, but there are no cameras registered!")))
		throw std::invalid_argument("Invalid camera state");

	return *(std::prev(m_priorityCameras.end())->second);
}

bool CameraController::TrySetCameraPriority(const CameraData& camera, const PriorityType& priority)
{
	const CameraData* targetCameraPtr = &camera;
	for (auto& priorityCamera : m_priorityCameras)
	{
		if (priorityCamera.second == targetCameraPtr)
		{
			CameraData* mutableCamera = priorityCamera.second;
			m_priorityCameras.extract(priorityCamera.first);
			m_priorityCameras.emplace(priority, mutableCamera);
			return true;
		}
	}
	return false;
}
bool CameraController::TrySetCurrentCamera(const CameraData& camera)
{
	return TrySetCameraPriority(camera, MAX_PRIORITY);
}