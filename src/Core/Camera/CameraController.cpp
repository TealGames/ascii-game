#include "pch.hpp"
#include "Core/Camera/CameraController.hpp"
#include "Utils/Debug.hpp"

CameraController::CameraController() : m_priorityCameras() {}

bool CameraController::Validate()
{
	return Assert(!m_priorityCameras.empty(), 
		"Validated camera controller but there are no cameras set which is not allowed");
}

bool CameraController::TryRegisterCamera(CameraComponent& camera, const PriorityType& priority)
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

CameraComponent& CameraController::GetActiveCameraMutable()
{
	if (!Assert(!m_priorityCameras.empty(), "Tried to get active camera MUTABLE from "
		"camera controller, but there are no cameras registered!"))
		throw std::invalid_argument("Invalid camera state");

	return *(std::prev(m_priorityCameras.end())->second);
}

const CameraComponent& CameraController::GetActiveCamera() const
{
	if (!Assert(!m_priorityCameras.empty(), "Tried to get active camera from "
		"camera controller, but there are no cameras registered!"))
		throw std::invalid_argument("Invalid camera state");

	return *(std::prev(m_priorityCameras.end())->second);
}

bool CameraController::TrySetCameraPriority(const CameraComponent& camera, const PriorityType& priority)
{
	const CameraComponent* targetCameraPtr = &camera;
	for (auto& priorityCamera : m_priorityCameras)
	{
		if (priorityCamera.second == targetCameraPtr)
		{
			CameraComponent* mutableCamera = priorityCamera.second;
			m_priorityCameras.extract(priorityCamera.first);
			m_priorityCameras.emplace(priority, mutableCamera);
			return true;
		}
	}
	return false;
}
bool CameraController::TrySetCurrentCamera(const CameraComponent& camera)
{
	return TrySetCameraPriority(camera, MAX_PRIORITY);
}