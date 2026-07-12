#include <cmath>
#include "pch.hpp"
#include "ECS/Systems/Types/World/LightSource2DSystem.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "Core/Primitives/Vector.hpp"
#include "Core/Scene/Scene.hpp"
#include "ECS/Systems/Types/World/EntityRenderer2DSystem.hpp"
#include "Core/Visual/TextBuffer.hpp"
#include "Core/Primitives/ColorGradient.hpp"
#include "Core/Scene/Scene.hpp"

#ifdef ENABLE_PROFILER
#include "Core/Analyzation/ProfilerTimer.hpp"
#endif 

namespace Engine::Lighting2D
{
    static constexpr bool CACHE_LAST_BUFFER = true;
    static constexpr bool STORE_LIGHT_MAP = true;

	LightSource2DSystem::LightSource2DSystem(const Rendering::EntityRenderer2DSystem& renderer) :
        m_rendererSystem(renderer)
	{
	}

    void LightSource2DSystem::SystemUpdate(Scenes::Scene& scene, Camera::CameraComponent& mainCamera, const float& deltaTime)
    {
#ifdef ENABLE_PROFILER
        ProfilerTimer timer("LightSourceSystem::SystemUpdate"); 
#endif 
        if (deltaTime <= 0) return;

        //TODO: fixed lighting points should bake their lighting into the buffer rather than have to reapply calculations
        //TODO: SPEEDUP could maybe be lighting that does not change (even if the lighting moves) could be made into lightmap
        //and then could apply lightmap to pixels around it rather than regenerating lighting data

        //TODO: optimization could be for us to not need to access text buffers directly and allow the scene to iteratoe
        //which can prevent the ned for pointers

        
        std::vector<Rendering::FragmentedTextBuffer2D*> affectedLayerBuffers = {};
        scene.OperateOnActiveComponents<LightSource2DComponent>(
            [this, &scene, &affectedLayerBuffers](LightSource2DComponent& data)-> void
            {
                //Log(LogType::Warning, std::format("Light data for {} is mutated: {}", entity.m_Name, std::to_string(data.m_MutatedThisFrame)));
                affectedLayerBuffers = scene.GetLayerBufferMutable(data.m_AffectedLayers);
              /*  if (CACHE_LAST_BUFFER && !data.m_MutatedThisFrame && !entity.m_Transform.HasMovedThisFrame()
                    && !data.m_LastFrameData.empty())
                {
                    LogWarning("LIGHT SOURCE READING LAST FRAME DATA");
                    scene.SetLayers(data.m_AffectedLayers, data.m_LastFrameData);
                    return;
                }*/

                //if (STORE_LIGHT_MAP && !data.m_MutatedThisFrame && !data.m_LightMap.empty())
                //{
                //   //Log(std::format("ALL light positions: {}", 
                //    //Utils::ToStringIterable<std::vector<LightMapChar>, LightMapChar>(data.m_LightMap)));
                //    LogWarning("LIGHT SOURCE CALCULATING FROM LIGHT MAP");
                //    Array2DPosition globalRowColPos = {};
                //    Array2DPosition entityRowColPos = Conversions::CartesianToArray(entity.m_Transform.m_Pos);
                //    Color newColor = {};

                //    //if (CACHE_LAST_BUFFER && !data.m_LastFrameData.empty()) data.m_LastFrameData.clear();
                //    int frameDataIndex = 0;
                //    for (int i=0; i<affectedLayerBuffers.size(); i++)
                //    {
                //        const auto& buffer = affectedLayerBuffers[i];
                //        for (const auto& lightMapChar : data.m_LightMap)
                //        {
                //            globalRowColPos = entityRowColPos + Conversions::GridToArray(lightMapChar.m_RelativePos);

                //           //Log(std::format("Relative pos: {} global: {} valid: {}",
                //           // lightMapChar.m_RelativeCartesianPos.ToString(), globalPos.ToString(), buffer->IsValidPos(globalPos)));
                //            if (!buffer->IsValidPos(globalRowColPos)) continue;
                //            const TextChar& currentTextChar = buffer->GetAtUnsafe(globalRowColPos);

                //            if (currentTextChar.m_Char == EMPTY_CHAR_PLACEHOLDER) continue;

                //            newColor = ApplyColorFilter(currentTextChar.m_Color, lightMapChar.m_FractionalFilterColor, lightMapChar.m_ColorFactor);
                //            Log(std::format("CREATING LIGHT MAP COLORS: old: {} factor: {} new: {}", 
                //                RaylibUtils::ToString(buffer->GetAtUnsafe(globalRowColPos).m_Color), std::to_string(lightMapChar.m_ColorFactor),
                //                RaylibUtils::ToString(newColor)));
                //            buffer->SetAt(globalRowColPos, newColor);

                //            //We only need to do one buffer since it will be the same map for all buffers
                //            if (CACHE_LAST_BUFFER && i==0)
                //            {
                //                if (frameDataIndex < data.m_LastFrameData.size()) 
                //                    data.m_LastFrameData[frameDataIndex] = TextCharPosition{ globalRowColPos, buffer->GetAtUnsafe(globalRowColPos) };
                //                else data.m_LastFrameData.emplace_back(globalRowColPos, buffer->GetAtUnsafe(globalRowColPos));
                //                //data.m_LastFrameData[index] = TextCharPosition({}, TextChar());
                //                frameDataIndex++;
                //            }
                //        }
                //    }
                //    return;
                //}

                scene.IncreaseFrameDirtyComponentCount();
                RenderLight(data, affectedLayerBuffers);
            });
            
    }

	void LightSource2DSystem::RenderLight(LightSource2DComponent& data, std::vector<Rendering::FragmentedTextBuffer2D*>& buffers, bool displayLightLevels)
    {
        //TODO: right now we use only the transform pos, but we should also use every pos on player too
        
        Rendering::EntityRenderer2DComponent* renderData = data.GetEntityMutable().TryGetComponentMutable<Rendering::EntityRenderer2DComponent>();
        if (!Assert(renderData != nullptr, "Tried to render light for entity: {} "
            "but could not find its entity render component!", data.GetEntity().m_Name)) 
            return;

        //TODO: it might not make sense for all lighting to just use the renderer to determine lighting start pos,
        //so perhaps this needs to be more customizable to allow for this and other behavior
        const Rendering::VisualData& visualData = renderData->GetVisualData();
       /* CartesianGridPosition centerPos = {};*/
       // std::cout << "REDNER LIGHT" << std::endl;

        for (auto& buffer : buffers)
        {
            if (buffer == nullptr) continue;
            CreateLightingForPoint(data, data.GetEntity().GetTransform().GetWorldPos(), *buffer, false);

            //Log(std::format("When rendering light start colors: {}", buffer->ToString(false)));
            //Log(std::format("Player Pos color: {}", RaylibUtils::ToString(buffer->GetAt(m_transform.m_Pos.GetFlipped())->m_Color)));
        }
    }

    //TODO: this probably needs to be optimized
    //TODO: there is a lot of get flopped and conversions from cartesia and row col pos so that could be optimized
    void LightSource2DSystem::CreateLightingForPoint(LightSource2DComponent& data,
        const WorldPosition3D& centerPos, Rendering::FragmentedTextBuffer2D& buffer, bool displayLightLevels)
    {
        std::sort(buffer.begin(), buffer.end(), 
            [&centerPos](const Rendering::TextBufferCharPosition2D& first, const Rendering::TextBufferCharPosition2D& second) -> bool
            {
                //Note: since we only care about the greater distance (and not actual value) we can avoid the expensive sqrt operation
                return (first.m_Pos - centerPos.GetXY()).GetMagnitudeSquared() < (second.m_Pos - centerPos.GetXY()).GetMagnitudeSquared();
            });

        float centerDistance = 0;
        for (auto& bufferPos : buffer)
        {
            centerDistance = (bufferPos.m_Pos - centerPos.GetXY()).GetMagnitude();
            if (centerDistance > data.m_LightRadius) break;

            bufferPos.m_Text.m_Color = CalculateNewColor(data, bufferPos, centerDistance, nullptr, nullptr);
        }
    }

    std::uint8_t LightSource2DSystem::CalculateLightLevelFromDistance(const LightSource2DComponent& data, const float& distance) const
    {
        //We do radius +1 since we want there to be 0 light when we go PAST the radius
        //NOTE: pow is expensive and can take 50-100 clock cycles
        return data.m_Intensity* std::powf(1 - (distance / (data.m_LightRadius + 1)), data.m_FalloffStrength);
    }

    ColHDR4 LightSource2DSystem::GetColorFromMultiplier(const ColHDR4& originalColor, const ColHDR4& filterColor, const float& colorMultiplier) const
    {
        return ColHDR4(std::roundf((originalColor.m_R) * (1 - colorMultiplier) + (filterColor.m_R) * (colorMultiplier)),
                     std::roundf((originalColor.m_G) * (1 - colorMultiplier) + (filterColor.m_G) * (colorMultiplier)),
                     std::roundf((originalColor.m_B) * (1 - colorMultiplier) + (filterColor.m_B) * (colorMultiplier)), 1.0f);
    }

    ColHDR4 LightSource2DSystem::CalculateNewColor(LightSource2DComponent& data,
        const Rendering::TextBufferCharPosition2D& bufferPos, const float& distance, std::uint8_t* outLightLevel, LightMapChar* lightMapChar) const
    {
        //Log(std::format("Distance between {} and {} is: {}",
        //currentPos.ToString(), centerPos.ToString(), std::to_string(distanceToCenter)));

        uint8_t lightLevel = CalculateLightLevelFromDistance(data, distance);
       // Log(std::format("Light level for distance: {} is: {}", std::to_string(distanceToCenter), std::to_string(lightLevel)));
        //std::cout << "Light level is: " << std::to_string(lightLevel) << std::endl;
        if (outLightLevel != nullptr) *outLightLevel = lightLevel;

        //We want to figure out the color data even if the pos is not valid in case we might need
        //light data to be stored even on invalid positions (so the light map can be created)
        const ColHDR4 filterColor = data.m_GradientFilter.GetColorAt(distance / data.m_LightRadius, false);
        const float colorMultiplier = static_cast<float>(lightLevel) / data.m_Intensity;
        //if (lightMapChar != nullptr) *lightMapChar = LightMapChar(centerPos - currentPos, RaylibUtils::GetFractionalColorRGB(filterColor, colorMultiplier), colorMultiplier);

        const ColHDR4 originalColor = bufferPos.m_Text.m_Color;
        const ColHDR4 newColor = GetColorFromMultiplier(originalColor, filterColor, colorMultiplier);

        //Log(std::format("Color multuplier for distance: {} (center {} -> {}) light level: {} is: {} new color: {}",
        //std::to_string(distanceToCenter), centerPos.ToString(), currentPos.ToString(),
        //std::to_string(lightLevel), std::to_string(colorMultiplier), RaylibUtils::ToString(originalColor)));
        return newColor;
    }
}
