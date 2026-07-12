#pragma once
#include <cmath>
#include "Utils/Math.hpp"
#include "Core/Primitives/WorldPosition.hpp"
#include "Core/Primitives/ScreenPosition.hpp"
#include "Core/Primitives/Vector.hpp"

namespace Engine::ECS { class EntityData; }
namespace Engine::Camera
{
	enum class ProjectionType : std::uint8_t
	{
		/// <summary>
		/// Perspective camera will make objects look 3D with a "vanishing" point.
		/// This means as you move farther away, the object looks smaller and like it "vanishes" into the distance
		/// </summary>
		Perspective = 0,
		/// <summary>
		/// Orthographic camera will make objects maintain the same size no matter how far away they 
		/// are from the camera. 
		/// </summary>
		Orthographic = 1
	};

	using EntityData = Engine::ECS::EntityData;
	class CameraSettings
	{
	private:
	public:
		const EntityData* m_FollowTarget;

		/// <summary>
		/// Aspect ratio of the viewport [WIDTH, HEIGHT] as width / height
		/// where this typically matches the screen space aspect ratio
		/// </summary>
		float m_AspectRatio;
		/// <summary>
		/// Controls the height of the camera viewport (where width is calculated from aspect ratio)
		/// </summary>
		//TODO: controlling viewport by multiplying aspect ratio by lens size feels akward find a better way
		float m_LensSize;

		constexpr static inline float DEFAULT_NEAR_DISTANCE = 0.1f;
		constexpr static inline float DEFAULT_FAR_DISTANCE = 1000;
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
		constexpr static inline float DEFAULT_FIELD_OF_VIEW_Y_RAD = ::Math::RAD_60;
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
		CameraSettings(const float aspectRatio, const float lensSize, const EntityData* followTarget = nullptr,
			const ProjectionType projection = DEFAULT_PROJECTION, const float nearDistance = DEFAULT_NEAR_DISTANCE,
			const float farDistance = DEFAULT_FAR_DISTANCE, const float fieldOfViewYRadians = DEFAULT_FIELD_OF_VIEW_Y_RAD);

		/// <summary>
		/// Calculates the viewport size of the camera (2D plane) at the camera depth (distance from camera forward dir)
		/// NOte: camera depth does not matter for orthographic, but is crucial for perspective projection
		/// Note: depth is regardless of near or far clipping plane -> it is just distance from camera,
		/// since far and near planes do not affect view size, just what is renderedcx
		/// </summary>
		/// <param name="cameraDepth"></param>
		/// <returns></returns>
		Vec2 CalculateViewportSize(const float cameraDepth = 0) const;

		float CalculateFovX() const;
		void SetFieldOfViewYDegrees(const float fovY);

		std::string ToString() const;
	};
}


