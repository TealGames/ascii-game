#include "Renderer.hpp"
#include <string>
#include <iostream>
#include <vector>
#include "raylib.h"
#include "TextBuffer.hpp"
#include "Point2DInt.hpp"
#include "Component.hpp"
#include "Transform.hpp"
#include "HelperFunctions.hpp"

namespace ECS
{
	Renderer::Renderer(const Transform& transform, TextBuffer& buffer,
		const std::vector<std::vector<TextChar>>& visualData) :
		m_transform(transform), m_outputBuffer(buffer), m_visualData(visualData), m_visualBoundsSize()
	{
		int maxWidth = 0;
		for (const auto& row : m_visualData)
		{
			if (row.size() > maxWidth) maxWidth = row.size();
		}
		m_visualBoundsSize = Utils::Point2DInt(m_visualData.size(), maxWidth);
	}

	UpdatePriority Renderer::GetUpdatePriority() const
	{
		return { MIN_PRIORITY+1 };
	}

	void Renderer::Start() {}
	void Renderer::UpdateStart(float deltaTime) 
	{
		//if (!m_transform.HasMovedThisFrame()) return;
		RenderInBuffer();
	}
	void Renderer::UpdateEnd(float deltaTime)
	{

	}

	const std::vector<std::vector<TextChar>>& Renderer::GetVisualData() const
	{
		return m_visualData;
	}

	std::string Renderer::GetVisualString() const
	{
		std::string visualStr = "";
		for (const auto& row : m_visualData)
		{
			for (const auto& col : row)
			{
				visualStr += col.m_Char;
			}
			visualStr += "\n";
		}
		return visualStr;
	}

	/// <summary>
	/// Returns the global position of the relative position of a segment of the visual using the
	/// current positon of the object.
	/// NOTE: RETURNS AS RENDER COORDS IN ROW, COL
	/// </summary>
	/// <param name="relativeVisualPos"></param>
	/// <returns></returns>
	Utils::Point2DInt Renderer::GetGlobalVisualPos(const Utils::Point2DInt& relativeVisualPos) const
	{
		Utils::Point2DInt centerBottom = m_transform.m_Pos.GetFlipped();

		Utils::Point2DInt bufferPos = {};
		bufferPos.m_X = centerBottom.m_X - m_visualBoundsSize.m_X + 1 + relativeVisualPos.m_X;
		bufferPos.m_Y = centerBottom.m_Y - (m_visualBoundsSize.m_Y / 2) + relativeVisualPos.m_Y;
		return bufferPos;
	}

	//TODO: this just get camera to find the current position within appearing rect and put that in buffer
	void Renderer::RenderInBuffer()
	{
		//Utils::Point2DInt half = {m_outputBuffer.m_HEIGHT/2, m_outputBuffer.m_WIDTH/2};
		//std::cout << "Rendering at: " << half.ToString() << std::endl;
		std::cout << "Rendering player" << std::endl;

		Utils::Point2DInt bufferPos = {};
		TextChar currentTextChar = {};
		for (int r = 0; r < m_visualData.size(); r++)
		{
			for (int c = 0; c < m_visualData[r].size(); c++)
			{
				bufferPos = GetGlobalVisualPos({ r, c });
				if (!m_outputBuffer.IsValidPos(bufferPos)) continue;

				currentTextChar = m_visualData[r][c];
				if (currentTextChar.m_Char == EMPTY_CHAR_PLACEHOLDER) continue;

				m_outputBuffer.SetAt(bufferPos, currentTextChar);
			}
		}
	}
}

