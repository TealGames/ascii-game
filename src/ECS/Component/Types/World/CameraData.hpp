#pragma once
#include <optional>
#include "Core/Camera/CameraSettings.hpp"
#include "ECS/Component/Component.hpp"
#include "Utils/Data/WorldPosition.hpp"
#include "Core/Visual/TextBuffer.hpp"
#include "Utils/Data/Matrix.hpp"
#include <vector>

class CameraData : public Component
{
public:
	CameraSettings m_CameraSettings;
	std::vector<TextBufferCharPosition> m_LastFrameBuffer;

public:
	CameraData();
	CameraData(const Json& json);
	CameraData(const CameraSettings& cameraSettings);

	void SetNearDistance(const float near);
	void SetFarDistance(const float far);

	float GetNearDistance() const;
	float GetFarDistance() const;

	ScreenPosition WorldToScreenPosition(const WorldPosition3D& pos) const;
	WorldPosition3D ScreenToWorldPosition(const ScreenPosition& pos) const;
	Vec2 WorldToScreenSize(const Vec2& worldSize) const;

	/// <summary>
	/// Gets the local forward direction of the camera. 
	/// May change based on the framework. (0, 0, -1) is the most common one
	/// used in OpenGL, Vulkan but DirectX uses (0, 0, 1)
	/// </summary>
	/// <returns></returns>
	Vec3 GetLocalForward() const;
	/// <summary>
	/// Gets the FORWARD direction of this camera using its rotation and 
	/// local forward direction
	/// </summary>
	/// <returns></returns>
	Vec3 CalculateWorldForward() const;

	WorldPosition3D CalculateNearPlaneWorldCenter() const;
	WorldPosition3D CalculateFarPlaneWorldCenter() const;

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