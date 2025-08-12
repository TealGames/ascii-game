#pragma once
#include "Utils/Data/WorldPosition.hpp"
#include "Utils/Data/ScreenPosition.hpp"
#include "Utils/Data/Vec2Type.hpp"
#include <cmath>

class EntityData;

enum class ProjectionType : std::uint8_t
{
	/// <summary>
	/// Perspective camera will make objects look 3D with a "vanishing" point.
	/// This means as you move farther away, the object looks smaller and like it "vanishes" into the distance
	/// </summary>
	Perspective =	0,
	/// <summary>
	/// Orthographic camera will make objects maintain the same size no matter how far away they 
	/// are from the camera. 
	/// </summary>
	Orthographic =	1
};

class CameraSettings
{
private:
public:
	const EntityData* m_FollowTarget;

	//Aspect ratio of the viewport in [WIDTH, HEIGHT]
	Vec2Int m_AspectRatio;
	/// <summary>
	/// Controls the width of the camera viewport (where height is calculated from aspect ratio)
	/// </summary>
	//TODO: controlling viewport by multiplying aspect ratio by lens size feels akward find a better way
	float m_LensSize;

	constexpr static inline float DEFAULT_NEAR_DISTANCE = 0.1f;
	constexpr static inline float DEFAULT_FAR_DISTANCE = 100;
	/// <summary>
	/// The farthest distance from the camera position that is rendered. 
	/// This value MUST BE POSITIVE REGARDLESS OF FORWARD DIR (AND GREATER THAN NEAR DISTANCE)
	/// </summary>
	float m_FarDistance;
	/// <summary>
	/// The closest distance from the camera position that is rendered.
	/// This value MUST BE POSITIVE REGARDLESS OF FORWARD DIR (AND SMALLER THAN FAR DISTANCE)
	/// AND MUST BE >0 OTHERWISE MATRIX CAN GET MESSED UP
	/// </summary>
	float m_NearDistance;

	constexpr static inline ProjectionType DEFAULT_PROJECTION = ProjectionType::Perspective;
	ProjectionType m_ProjectionType;

	//Default is 60 degrees
	constexpr static inline float DEFAULT_FIELD_OF_VIEW_Y_RAD = 3.14159 / 2;
	/// <summary>
	/// The field of view Y angle (from top to bottom) -> how much the player 
	/// sees vertically. It is easier to use y fov because it can remain the same 
	/// as aspect ratio changes because vertical black side bars (pillarboxing) being added due to aspect ratio
	/// changes are often avoided since it is less natural comapared to allowing an increase in horizontal fov
	/// which increases peripheral vision -> more natural to human vision
	/// </summary>
	float m_FieldOfViewYRadians;

private:
public:
	CameraSettings();
	CameraSettings(const Vec2Int& aspectRatio, const float& lensSize, const EntityData* followTarget=nullptr, 
		const ProjectionType projection= DEFAULT_PROJECTION, const float nearDistance= DEFAULT_NEAR_DISTANCE, 
		const float farDistance= DEFAULT_FAR_DISTANCE, const float fieldOfViewYRadians= DEFAULT_FIELD_OF_VIEW_Y_RAD);

	/// <summary>
	/// Calculates the viewport size of the camera (2D plane) at the camera depth (distance from camera forward dir)
	/// NOte: camera depth does not matter for orthographic, but is crucial for perspective projection
	/// Note: depth is regardless of near or far clipping plane -> it is just distance from camera,
	/// since far and near planes do not affect view size, just what is renderedcx
	/// </summary>
	/// <param name="cameraDepth"></param>
	/// <returns></returns>
	WorldPosition2D CalculateViewportSize(const float cameraDepth=0) const;

	float CalculateAspectRatio() const;
	float CalculateFovX() const;

	void SetFieldOfViewYDegrees(const float fovY);

	std::string ToString() const;
};

