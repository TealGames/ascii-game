#pragma once
#include <map>
#include "ECS/Component/Types/World/CameraData.hpp"
#include "Core/IValidateable.hpp"
#include <cstdint>
#include <limits>

using PriorityType = std::uint8_t;
class CameraController : public IValidateable
{
private:
	std::multimap<PriorityType, CameraComponent*> m_priorityCameras;
public:
	static constexpr PriorityType MIN_PRIORITY = 0;
	static constexpr PriorityType MAX_PRIORITY = std::numeric_limits<PriorityType>::max();

private:
public:
	CameraController();

	bool Validate() override;

	bool TryRegisterCamera(CameraComponent& camera, const PriorityType& priority= MIN_PRIORITY);
	void UpdateActiveCamera();

	CameraComponent& GetActiveCameraMutable();
	const CameraComponent& GetActiveCamera() const;

	bool TrySetCameraPriority(const CameraComponent& camera, const PriorityType& priority);
	bool TrySetCurrentCamera(const CameraComponent& camera);
};

