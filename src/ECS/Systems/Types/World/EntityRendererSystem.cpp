#include "pch.hpp"
#include "ECS/Systems/Types/World/EntityRendererSystem.hpp"
#include "Core/Visual/TextBuffer.hpp"
#include "Utils/Data/Array2DPosition.hpp"
#include "ECS/Component/Component.hpp"
#include "ECS/Systems/Types/World/TransformSystem.hpp"
#include "Utils/HelperFunctions.hpp"
#include "Core/PositionConversions.hpp"
#include "Core/Scene/Scene.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "Core/Rendering/GameRenderer.hpp"
#include "Core/Asset/FontAsset.hpp"

#ifdef ENABLE_PROFILER
#include "Core/Analyzation/ProfilerTimer.hpp"
#endif 

namespace ECS
{
	//TODO: cache last buffer is not implemneted (but should maybe be removed since there 
	//is now way to know what texture is for what entity so we cant change it)
	static constexpr bool CACHE_LAST_BUFFER = true;

	EntityRendererSystem::EntityRendererSystem(Rendering::Renderer& renderer) : m_renderer(&renderer)
	{
		
	}

	void EntityRendererSystem::SystemUpdate(Scene& scene, CameraComponent& mainCamera, const float& deltaTime)
	{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("EntityRendererSystem::SystemUpdate");
#endif 

		//TODO: create a map or vector of all layer pointers, so we do not have to reget them for each entity since the layers do not change
		//AKA: memoization
		std::vector<std::tuple<RenderLayerType, FragmentedTextBuffer2D*>> allLayerBuffers = scene.GetAllLayerBufferMutable();
		FragmentedTextBuffer2D* currLayerBuffer = nullptr;
		scene.OperateOnComponents<EntityRendererData>(
			[this, &scene, &allLayerBuffers, &currLayerBuffer, &mainCamera](EntityRendererData& data)-> void
			{
				AddTextToRenderer(data, mainCamera);

				//for (const auto& renderLayerTuple : allLayerBuffers)
				//{
				//	if (!Utils::HasFlagAll(data.GetRenderLayers(), std::get<0>(renderLayerTuple)))
				//		continue;

				//	currLayerBuffer = std::get<1>(renderLayerTuple);
				//	if (currLayerBuffer == nullptr)
				//	{
				//		LogError(std::format("Tried to update render system "
				//			"but entity's render data: {} found a NULL render layer buffer", data.GetEntity().m_Name));
				//		return;
				//	}
				//		
				//	AddTextToRenderer(*currLayerBuffer, data, mainCamera);
				//	//LogWarning(std::format("Entity: {} has been added to buffer. new buffer: {}", entity.m_Name, ToString(*buffer)));
				//}
			});
	}

	std::string EntityRendererSystem::GetVisualString(const EntityRendererData& data) const
	{
		std::string visualStr = "";
		return data.GetVisualData().ToString();
	}

	void EntityRendererSystem::AddTextToRenderer(EntityRendererData& data, const CameraComponent& mainCamera)
	{
		const float zPos = data.GetTransform().GetGlobalPos().m_Z;
		const VisualData& visual = data.GetVisualData();
		const auto& visualBuffer = visual.GetBuffer();
		const WorldPosition3D pivotWorldPos = data.GetVisualData().GetPivotWorldPos(data.GetTransform().GetGlobalPos());

		for (const auto& charPos : visualBuffer)
		{
			if (!mainCamera.DoesViewVolumeContainPos(Vec3(charPos.m_Pos, zPos)))
				continue;

			if (!charPos.m_FontData.HasValidFont())
			{
				LogError(std::format("Attempted to add text positions to buffer "
					"but found invalid font on char pos:{}", charPos.ToString()));
				return;
			}

			//TODO: add each char as a renderer text call
			//m_renderer->AddTextCall(data.GetTransform().GetLocalPos(), charPos.m_FontData.m_FontAsset->GetFont(), charPos.m_Text.m_Char, charPos.m_FontData.m_)
		}
	}
}

