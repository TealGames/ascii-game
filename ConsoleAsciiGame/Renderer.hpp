#pragma once
#include <string>
#include <vector>
#include "raylib.h"
#include "TextBuffer.hpp"
#include "Point2DInt.hpp"
#include "Component.hpp"
#include "Transform.hpp"
#include "TextBuffer.hpp"

namespace ECS
{
	//struct VisualData
	//{
	//	//The position of the char data relative to the position of the character
	//	Utils::Point2DInt CharRelativePos;
	//	TextChar CharData;
	//};

	class Renderer: public Component
	{
	private:
		const Transform& m_transform;
		TextBuffer& m_outputBuffer;

		//Stores every entity's visual from top row to bottom row
		const std::vector<std::vector<TextChar>> m_visualData;

		//The bounding box's size for this visual in (HEIGHT, WIDTH)
		Utils::Point2DInt m_visualBoundsSize;

	public:

	private:
		void RenderInBuffer();

	public:
		Renderer(const Transform& transform, TextBuffer& buffer, 
			const std::vector<std::vector<TextChar>>& visualData);

		UpdatePriority GetUpdatePriority() const override;

		void Start() override;
		void Update(float deltaTime) override;

		Utils::Point2DInt GetGlobalVisualPos(const Utils::Point2DInt& relativeVisualPos) const;

		const std::vector<std::vector<TextChar>>& GetVisualData() const;
		std::string GetVisualString() const;
	};
}


