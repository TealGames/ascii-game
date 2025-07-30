#pragma once
#include <optional>
#include "Core/Camera/CameraSettings.hpp"
#include "ECS/Component/Component.hpp"
#include "Utils/Data/WorldPosition.hpp"
#include "Core/Visual/TextBuffer.hpp"
#include "Math/Ray.hpp"
#include "Math/Plane.hpp"
#include "Math/PlatformMath.hpp"
#include <vector>

class CameraData : public Component
{
public:
	CameraSettings m_CameraSettings;
	std::vector<TextBufferCharPosition2D> m_LastFrameBuffer;

private:
	WorldPosition3D CalculateNearPlaneWorldCenter(const WorldPosition3D& globalPos) const;
	WorldPosition3D CalculateFarPlaneWorldCenter(const WorldPosition3D& globalPos) const;
public:
	CameraData();
	CameraData(const Json& json);
	CameraData(const CameraSettings& cameraSettings);

	void SetNearDistance(const float near);
	void SetFarDistance(const float far);

	float GetNearDistance() const;
	float GetFarDistance() const;
	float GetViewDistance() const;

	ScreenPosition WorldToScreenPosition(const WorldPosition3D& pos) const;
	/// <summary>
	/// Converts screen position. Top left is (0,0) bottom right is (screen width, screen height)
	/// X -> increases RIGHT, y-> increases DOWN
	/// </summary>
	/// <param name="pos"></param>
	/// <returns></returns>
	Ray3D ScreenToWorldPosition(const ScreenPosition& pos) const;
	Vec2 WorldToScreenSize(const float cameraDepthDistance, const Vec3& worldSize) const;

	/// <summary>
	/// Gets the FORWARD direction of this camera using its rotation and 
	/// local forward direction
	/// </summary>
	/// <returns></returns>
	Vec3 CalculateWorldForward() const;

	WorldPosition3D CalculateNearPlaneWorldCenter() const;
	WorldPosition3D CalculateFarPlaneWorldCenter() const;

	/// <summary>
	/// Calculates the planes that make up the view frustum in the order:
	/// LEFT, RIGHT, BOTTOM, TOP, NEAR, FAR
	/// </summary>
	/// <returns></returns>
	std::array<InfinitePlane3D, 6> CalculateFrustumPlanes() const;
	/// <summary>
	/// Checks whether the point is within the view volume/frustum 
	/// and can be rendered. Provide the precalculated planes to prevent
	/// unnecessary plane calculates if this is called many times
	/// </summary>
	/// <param name="point"></param>
	/// <param name="precalculatedPlanes"></param>
	/// <returns></returns>
	bool DoesViewVolumeContainPosOptimized(const WorldPosition3D& point, 
		const std::array<InfinitePlane3D, 6>* precalculatedPlanes) const;
	bool DoesViewVolumeContainPos(const WorldPosition3D& point) const;

	/// <summary>
	/// Calculates the view matrix that converts world space into camera's view.
	/// It essentially moves the world into the camera's view
	/// </summary>
	/// <returns></returns>
	Mat4 CalculateViewMatrix() const;
	/// <summary>
	/// Calculates the camera's view space into 2d plane screen space
	/// based on the projection type that is used
	/// </summary>
	/// <returns></returns>
	Mat4 CalculateProjectionMatrix() const;

	//std::vector<std::string> GetDependencyFlags() const override;
	void InitFields() override;

	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
};