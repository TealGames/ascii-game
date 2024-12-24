#pragma once
#include "Component.hpp"
#include "Point2DInt.hpp"

namespace ECS
{
	class Transform : public Component
	{
	private:
		Utils::Point2DInt m_pos;
		Utils::Point2DInt m_lastPos;
		Utils::Point2DInt m_lastFramePos;

	public:
		const Utils::Point2DInt& m_Pos;

		//Since negative positions are not allowed
		static const Utils::Point2DInt NULL_POS;

	private:
	public:
		Transform(const Utils::Point2DInt& startPos);
		UpdatePriority GetUpdatePriority() const override;

		void Start() override;
		void UpdateStart(float deltaTime) override;
		void UpdateEnd(float deltaTime) override;

		void SetPos(const Utils::Point2DInt& newPos);
		void SetPosX(const int& newX);
		void SetPosY(const int& newY);

		void SetDeltaX(const int& xDelta);
		void SetDeltaY(const int yDelta);

		bool HasMovedThisFrame() const;
	};
}


