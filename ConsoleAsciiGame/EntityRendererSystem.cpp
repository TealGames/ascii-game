#include "pch.hpp"
#include "raylib.h"
#include "EntityRendererSystem.hpp"
#include "TextBuffer.hpp"
#include "Point2DInt.hpp"
#include "Array2DPosition.hpp"
#include "Component.hpp"
#include "TransformSystem.hpp"
#include "HelperFunctions.hpp"
#include "PositionConversions.hpp"

namespace ECS
{
	static constexpr bool CACHE_LAST_BUFFER = true;

	EntityRendererSystem::EntityRendererSystem(const TransformSystem& transform) : 
		m_transformSystem(transform)
	{
		
	}

	void EntityRendererSystem::SystemUpdate(Scene& scene, const float& deltaTime)
	{
		std::vector<TextBuffer*> affectedLayerBuffers = {};
		scene.OperateOnComponents<EntityRendererData>(
			[this, &scene, &affectedLayerBuffers](EntityRendererData& data, ECS::Entity& entity)-> void
			{
				//Utils::Log(std::format("Player is at pos: {}", entity.m_Transform.m_Pos.ToString()));
				affectedLayerBuffers = scene.GetTextBuffersMutable(data.m_RenderLayers);
				//Utils::Log(std::format("RENDER LAYERS: {}", std::to_string(affectedLayerBuffers.size())));

				//Utils::Log(std::format("Moved this frame: {}", std::to_string(m_transformSystem.HasMovedThisFrame(entity.m_Transform))));

				if (!Utils::Assert(!affectedLayerBuffers.empty(), std::format("Tried to update render system "
					"but entity's render data: {} has no render layers", entity.m_Name))) return;

				if (CACHE_LAST_BUFFER && !data.m_MutatedThisFrame && !m_transformSystem.HasMovedThisFrame(entity.m_Transform) && 
					!data.m_LastFrameVisualData.empty())
				{
					for (auto& buffer : affectedLayerBuffers)
					{
						if (buffer == nullptr) continue;
						/*Utils::Log(std::format("STARTING BUFFER: {}       -> ALL VISUAL DATA: {}",
							buffer->ToString(), Utils::ToStringIterable<std::vector<TextCharPosition>, TextCharPosition>(data.m_LastFrameVisualData)));*/

						for (const auto& data : data.m_LastFrameVisualData)
							buffer->SetAt(data.m_RowColPos, data.m_Text);
					}
					return;
				}

				scene.IncreaseFrameDirtyComponentCount();
				if (!data.m_LastFrameVisualData.empty())
					data.m_LastFrameVisualData.clear();

				for (auto& buffer : affectedLayerBuffers)
				{
					if (!Utils::Assert(buffer != nullptr, std::format("Tried to update render system "
						"but entity's render data: {} found a NULL render layer buffer", entity.m_Name))) return;

					RenderInBuffer(*buffer, data, entity);
				}
				data.m_MutatedThisFrame = false;
			});
	}

	void EntityRendererSystem::SetVisualData(EntityRendererData& data, const VisualData& visual)
	{
		data.m_VisualData = visual;
	}
	void EntityRendererSystem::SetVisualData(EntityRendererData& data, const VisualDataPositions& positions)
	{
		data.m_VisualData.SetAt(positions.m_Data);
	}

	const std::vector<std::vector<TextChar>>& EntityRendererSystem::GetVisualData(const EntityRendererData& data) const
	{
		return data.m_VisualData.GetFull();
	}

	std::string EntityRendererSystem::GetVisualString(const EntityRendererData& data) const
	{
		std::string visualStr = "";
		for (const auto& row : GetVisualData(data))
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
	Array2DPosition EntityRendererSystem::GetGlobalVisualPos(const Array2DPosition& relativeVisualPos,
		const EntityRendererData& data, const Entity& entity) const
	{
		Array2DPosition centerBottom = Conversions::CartesianToArray(entity.m_Transform.m_Pos);

		Array2DPosition bufferPos = {};
		bufferPos.SetRow(centerBottom.GetRow() - data.m_VisualBoundsSize.m_X + 1 + relativeVisualPos.GetRow());
		bufferPos.SetCol(centerBottom.GetCol() - (data.m_VisualBoundsSize.m_Y / 2) + relativeVisualPos.GetCol());
		return bufferPos;
	}

	//TODO: this just get camera to find the current position within appearing rect and put that in buffer
	void EntityRendererSystem::RenderInBuffer(TextBuffer& buffer, EntityRendererData& data, const Entity& entity)
	{
		//Utils::Point2DInt half = {m_outputBuffer.m_HEIGHT/2, m_outputBuffer.m_WIDTH/2};
		//std::cout << "Rendering at: " << half.ToString() << std::endl;
		//std::cout << "Rendering player" << std::endl;

		//Utils::Log(std::format("Rendering player at; {}", entity.m_Transform.m_Pos.ToString()));
		Array2DPosition bufferPos = {};
		TextChar currentTextChar = {};
		const auto& fullData = data.m_VisualData.GetFull();
		for (int r = 0; r < fullData.size(); r++)
		{
			for (int c = 0; c < fullData[r].size(); c++)
			{
				bufferPos = GetGlobalVisualPos({ r, c }, data, entity);
				if (!buffer.IsValidPos(bufferPos)) continue;

				currentTextChar = fullData[r][c];
				if (currentTextChar.m_Char == EMPTY_CHAR_PLACEHOLDER) continue;

				buffer.SetAt(bufferPos, currentTextChar);
				//Utils::Log(std::format("Setting entity buffer render: {}", bufferPos.ToString()));
				if (CACHE_LAST_BUFFER) data.m_LastFrameVisualData.emplace_back(bufferPos, currentTextChar);
			}
		}
	}
}

