#pragma once
#include "Core/Camera/CameraSettings.hpp"
#include "ECS/Component/Component.hpp"
#include "Core/Primitives/WorldPosition.hpp"
#include "Math/Ray.hpp"
#include "Math/Plane.hpp"
#include "Core/Primitives/Matrix.hpp"
#include "Math/Quaternion.hpp"
#include "Utils/HelperMacros.hpp"

namespace Engine::Camera
{
	enum class ProjectionMatrixType : std::uint8_t
	{
		Platform = 0,
		Engine = 1
	};

	enum class CameraPrecalculatedDataUpdate : std::uint8_t
	{
		None = 0,
		ViewMatrix = 1,
		PlatformProjMatrix = 1 << 1,
		EngineProjMatrix = 1 << 2,
		FrustumPlanes = 1 << 3,
		All = 0xFF
	};
	FLAG_ENUM_OPERATORS(CameraPrecalculatedDataUpdate)

	struct CameraPrecalculatedData
	{
		Mat4 m_ViewMatrix = {};
		Mat4 m_PlatformProjectionMatrix = {};
		Mat4 m_EngineProjectionMatrix = {};
		std::array<Math::InfinitePlane3D, 6> m_FrustumPlanes = {};
		CameraPrecalculatedDataUpdate m_UpdatedThisFrame;

		std::string ToString() const;
	};

	constexpr Vec3 INVALID_NDC_POS = { -2, -2, -2 };
	class CameraSystem;
	class CameraComponent : public ECS::Component
	{
	private:
		mutable CameraPrecalculatedData m_lastUpdateData;
		CameraSettings m_cameraSettings;
	public:
		friend class CameraSystem;

		static constexpr ECS::DirtyFlag VIEW_MATRIX_DIRTY_FLAG = 1;
		static constexpr ECS::DirtyFlag PROJ_MATRIX_DIRTY_FLAG = 1 << 1;

	private:
		/// <summary>
		/// Calculates the planes that make up the view frustum in the order:
		/// LEFT, RIGHT, BOTTOM, TOP, NEAR, FAR
		/// </summary>
		/// <returns></returns>
		std::array<Math::InfinitePlane3D, 6> CalculateFrustumPlanes() const;

		/// <summary>
		/// Calculates the camera's view space into 2d plane screen space
		/// based on the projection type that is used FOR THE CURRENTLY
		/// USED RENDERING PLATFORM (Platform) OR for this engine (ENGINE)
		/// </summary>
		/// <returns></returns>
		Mat4 CalculateProjectionMatrix(const ProjectionMatrixType type) const;

	public:
		CameraComponent();
		CameraComponent(const CameraSettings& cameraSettings);

		const CameraSettings& GetSettings() const;
		void SetFollowNoTarget();
		void SetFollowTarget(const EntityData& entity);
		bool HasFollowTarget() const;
		const EntityData* GetFollowTarget() const;

		void SetNearDistance(const float near);
		void SetFarDistance(const float far);

		float GetNearDistance() const;
		float GetFarDistance() const;
		float GetViewDistance() const;

		ScreenPosition WorldToScreenPosition(const WorldPosition3D& pos) const;
		/// <summary>
		/// Will get the world position to its normalized device coor
		/// </summary>
		/// <param name="pos"></param>
		/// <returns></returns>
		Vec3 WorldToNdcPosition(const WorldPosition3D& pos, const ProjectionMatrixType matrixType) const;
		/// <summary>
		/// Converts screen position. Top left is (0,0) bottom right is (screen width, screen height)
		/// X -> increases RIGHT, y-> increases DOWN
		/// </summary>
		/// <param name="pos"></param>
		/// <returns></returns>
		Math::Ray3D ScreenToWorldPosition(const ScreenPosition& pos) const;
		Vec2 WorldToScreenSize(const float cameraDepthDistance, const Vec3& worldSize) const;

		WorldPosition3D CalculateNearPlaneWorldCenter() const;
		WorldPosition3D CalculateFarPlaneWorldCenter() const;
		bool DoesViewVolumeContainPos(const WorldPosition3D& point) const;

		//void UpdatePrecalculatedData();
		const CameraPrecalculatedData& GetLastUpdateData() const;

		//std::vector<std::string> GetDependencyFlags() const override;
		void InitFields() override;
		void Serialize(Serialization::Serializer& serializer) const override;
		void Deserialize(Serialization::Deserializer& deserializer) override;
		std::string ToString() const override;
	};

	/// <summary>
	/// Calculates the view matrix that converts world space into camera's view.
	/// It essentially moves the world into the camera's view
	/// </summary>
	/// <returns></returns>
	Mat4 CalculateViewMatrix(const WorldPosition3D& globalPos, const Math::Quat& globalRotation);
	Mat4 CalculateViewMatrix(const WorldPosition3D& globalPos, const Vec3& forwardDir, const Vec3& upDir);
}
