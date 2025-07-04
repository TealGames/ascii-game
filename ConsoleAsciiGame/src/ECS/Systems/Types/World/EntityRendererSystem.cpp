#include "pch.hpp"
#include "raylib.h"
#include "ECS/Systems/Types/World/EntityRendererSystem.hpp"
#include "Core/Visual/TextBuffer.hpp"
#include "Utils/Data/Array2DPosition.hpp"
#include "ECS/Component/Component.hpp"
#include "ECS/Systems/Types/World/TransformSystem.hpp"
#include "Utils/HelperFunctions.hpp"
#include "Core/PositionConversions.hpp"
#include "Core/Scene/Scene.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"

#ifdef ENABLE_PROFILER
#include "Core/Analyzation/ProfilerTimer.hpp"
#endif 

namespace ECS
{
	//TODO: cache last buffer is not implemneted (but should maybe be removed since there 
	//is now way to know what texture is for what entity so we cant change it)
	static constexpr bool CACHE_LAST_BUFFER = true;

	EntityRendererSystem::EntityRendererSystem()
	{
		
	}

	void EntityRendererSystem::SystemUpdate(Scene& scene, CameraData& mainCamera, const float& deltaTime)
	{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("EntityRendererSystem::SystemUpdate");
#endif 

		std::vector<FragmentedTextBuffer*> affectedLayerBuffers = {};
		scene.OperateOnComponents<EntityRendererData>(
			[this, &scene, &affectedLayerBuffers](EntityRendererData& data)-> void
			{
				//if(entity.GetName()== "Background") Assert(false, std::format("Entity:{} has visual:{}", entity.GetName(), data.m_VisualData.ToString()));

				//Log(std::format("Player is at pos: {}", entity.m_Transform.m_Pos.ToString()));
				affectedLayerBuffers = scene.GetLayerBufferMutable(data.GetRenderLayers()); 
				//Log(std::format("RENDER LAYERS: {}", std::to_string(affectedLayerBuffers.size())));

				//Log(std::format("Moved this frame: {}", std::to_string(m_transformSystem.HasMovedThisFrame(entity.m_Transform))));
				//LogError(std::format("render system update entity: {} SCENE:{}", entity.ToString(), scene.ToString()));
				//Assert(false, std::format("BALLS"));
				//return;
				//LogError(std::format("Trying to render obkect: {}", entity.GetName()));
				//return;

				if (!Assert(!affectedLayerBuffers.empty(), std::format("Tried to update render system "
					"but entity's render data: {} has no render layers", data.GetEntity().m_Name))) return;

				/*
				if (CACHE_LAST_BUFFER && !data.m_MutatedThisFrame && !entity.m_Transform.HasMovedThisFrame() &&
					!data.m_LastFrameVisualData.empty())
				{
					//for (auto& buffer : affectedLayerBuffers)
					//{
					//	if (buffer == nullptr) continue;
					//	/*Log(std::format("STARTING BUFFER: {}       -> ALL VISUAL DATA: {}",
					//		buffer->ToString(), Utils::ToStringIterable<std::vector<TextCharPosition>, TextCharPosition>(data.m_LastFrameVisualData)));

					//	buffer->m_TexturePositions = data.m_LastFrameVisualData;
					//}
					//return;
				}
				*/

				scene.IncreaseFrameDirtyComponentCount();
				/*if (!data.m_LastFrameVisualData.empty())
					data.m_LastFrameVisualData.clear();*/

				//LogWarning(std::format("Entity: {} has visual: {}", entity.m_Name, data.GetVisualData().m_Text.ToString()));
				for (auto& buffer : affectedLayerBuffers)
				{
					if (!Assert(buffer != nullptr, std::format("Tried to update render system "
						"but entity's render data: {} found a NULL render layer buffer", data.GetEntity().m_Name))) return;

					AddTextToBuffer(*buffer, data);
					//LogWarning(std::format("Entity: {} has been added to buffer. new buffer: {}", entity.m_Name, ToString(*buffer)));
				}
				data.m_MutatedThisFrame = false;
			});
	}

	std::string EntityRendererSystem::GetVisualString(const EntityRendererData& data) const
	{
		std::string visualStr = "";
		return data.GetVisualData().ToString();
	}

	void EntityRendererSystem::AddTextToBuffer(FragmentedTextBuffer& buffer, EntityRendererData& data)
	{
		//TODO: should this really be a function of visual data and should we expose the buffer directly from the scene like this
		data.GetVisualData().AddTextPositionsToBuffer(data.GetEntity().GetTransform().GetGlobalPos(), buffer);
	}
}

