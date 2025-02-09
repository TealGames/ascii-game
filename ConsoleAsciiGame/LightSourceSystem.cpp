#include <numbers>
#include <limits>
#include <cmath>

#include "pch.hpp"
#include "LightSourceSystem.hpp"
#include "Component.hpp"
#include "Point2D.hpp"
#include "CartesianPosition.hpp"
#include "Array2DPosition.hpp"
#include "PositionConversions.hpp"
#include "Point4D.hpp"
#include "Scene.hpp"
#include "EntityRendererSystem.hpp"
#include "TransformSystem.hpp"
#include "TextBuffer.hpp"
#include "HelperFunctions.hpp"
#include "RaylibUtils.hpp"
#include "ColorGradient.hpp"
#include "ProfilerTimer.hpp"

namespace ECS
{
    static constexpr bool CACHE_LAST_BUFFER = true;
    static constexpr bool STORE_LIGHT_MAP = true;

	LightSourceSystem::LightSourceSystem(const EntityRendererSystem& renderer) :
        m_rendererSystem(renderer)
	{
	}

    void LightSourceSystem::SystemUpdate(Scene& scene, const float& deltaTime)
    {
#ifdef ENABLE_PROFILER
        ProfilerTimer timer("LightSourceSystem::SystemUpdate");
#endif 

        //TODO: fixed lighting points should bake their lighting into the buffer rather than have to reapply calculations
        //TODO: SPEEDUP could maybe be lighting that does not change (even if the lighting moves) could be made into lightmap
        //and then could apply lightmap to pixels around it rather than regenerating lighting data

        //TODO: optimization could be for us to not need to access text buffers directly and allow the scene to iteratoe
        //which can prevent the ned for pointers

        
        std::vector<TextBufferMixed*> affectedLayerBuffers = {};
        scene.OperateOnComponents<LightSourceData>(
            [this, &scene, &affectedLayerBuffers](LightSourceData& data, ECS::Entity& entity)-> void
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
                if (CACHE_LAST_BUFFER && !data.m_LastFrameData.empty()) data.m_LastFrameData.clear();
                RenderLight(data, entity, affectedLayerBuffers);
                data.m_MutatedThisFrame = false;
                //std::cout << "Rendering lgiht" << std::endl;
            });
            
    }

	void LightSourceSystem::RenderLight(LightSourceData& data, ECS::Entity& entity, 
        std::vector<TextBufferMixed*>& buffers, bool displayLightLevels)
    {
        //TODO: right now we use only the transform pos, but we should also use every pos on player too
        
        EntityRendererData* renderData = entity.TryGetComponent<EntityRendererData>();
        if (!Assert(renderData != nullptr, std::format("Tried to render light for entity: {} "
            "but could not find its entity render component!", entity.m_Name))) return;

        //TODO: it might not make sense for all lighting to just use the renderer to determine lighting start pos,
        //so perhaps this needs to be more customizable to allow for this and other behavior
        const VisualData& visualData = renderData->GetVisualData();
        CartesianGridPosition centerPos = {};
       // std::cout << "REDNER LIGHT" << std::endl;

        for (auto& buffer : buffers)
        {
            if (buffer == nullptr) continue;
            CreateLightingForPoint(data, entity, entity.m_Transform.m_Pos, *buffer, false);

            //Log(std::format("When rendering light start colors: {}", buffer->ToString(false)));
            //Log(std::format("Player Pos color: {}", RaylibUtils::ToString(buffer->GetAt(m_transform.m_Pos.GetFlipped())->m_Color)));

            /*
            for (int r = 0; r < visualData.m_Text.GetHeight(); r++)
            {
                for (int c = 0; c < visualData.m_Text.GetWidth(); c++)
                {
                    //Implement how to find the global pos of a point of visual data
                    CreateLightingForPoint(data, entity, );
                }
            }
            */
        }
    }

    //TODO: this probably needs to be optimized
    //TODO: there is a lot of get flopped and conversions from cartesia and row col pos so that could be optimized
    void LightSourceSystem::CreateLightingForPoint(LightSourceData & data, const ECS::Entity & entity,
        const WorldPosition& centerPos, TextBufferMixed & buffer, bool displayLightLevels)
    {
        std::sort(buffer.begin(), buffer.end(), 
            [&centerPos](const TextBufferPosition& first, const TextBufferPosition& second) -> bool
            {
                return Utils::GetDistance(centerPos, first.m_Pos) < Utils::GetDistance(centerPos, second.m_Pos);
                
            });

        float centerDistance = 0;
        for (auto& bufferPos : buffer)
        {
            centerDistance = Utils::GetDistance(centerPos, bufferPos.m_Pos);
            if (centerDistance > data.m_LightRadius) break;

            bufferPos.m_Text.m_Color = CalculateNewColor(data, entity, bufferPos, centerDistance, nullptr, nullptr);
        }
        /*
        if (!data.m_LightMap.empty()) data.m_LightMap.clear();

        //std::cout << "Center pos: " << centerPos.ToString() << std::endl;
        int radius = data.m_LightRadius;
        const CartesianGridPosition maxXY = { centerCartesianPos.m_X + radius, centerCartesianPos.m_Y + radius };
        const CartesianGridPosition minXY = { centerCartesianPos.m_X - radius, centerCartesianPos.m_Y - radius };

        //We approxiamte how accurate the circle should be by using the total area to figure out how many points we need
        const int positiveSidePoints = std::min(buffer.GetHeight(), buffer.GetWidth()) / radius;
        const float pointXValIncrement = 0.5f;

        //We add all the points on circle using X, Y Coords
        float x = minXY.m_X;
        float y = 0;
        std::vector<CartesianGridPosition> bottomCirclePoints = {};
        while (x <= maxXY.m_X)
        {
            y = std::sqrt(std::pow(data.m_LightRadius, 2) - std::pow(x - centerCartesianPos.m_X, 2)) + centerCartesianPos.m_Y;
            bottomCirclePoints.push_back(CartesianGridPosition(x, y));

            x += pointXValIncrement;
        }
        //Log(std::format("Circle points: {}", Utils::ToStringIterable<std::vector<Utils::Point2D>, Utils::Point2D>(bottomCirclePoints)));
        
        const CartesianGridPosition entityCartesianPos = Conversions::CartesianToGrid(entity.m_Transform.m_Pos);

        Color newColor = {};
        CartesianGridPosition currentXYCoord = {};
        std::vector<CartesianGridPosition> seenCoords = {};
        std::uint8_t lightLevel = MIN_LIGHT_LEVEL;
        LightMapChar lightMapChar = {};
        std::string lightLevelStr = "";
        std::optional<int> prevLevel = std::nullopt;
        bool isInitialPosValid = false;

        for (const auto& point : bottomCirclePoints)
        {
            if (point.m_X <= centerCartesianPos.m_X) currentXYCoord.m_X = std::floor(point.m_X);
            else currentXYCoord.m_X = std::ceil(point.m_X);

            currentXYCoord.m_Y = std::ceil(point.m_Y);
            isInitialPosValid = buffer.IsValidPos(Conversions::GridToArray(currentXYCoord));

            //We need to check if it does not have them already since we may have a very short range with many points
            //which could result in potential duplicates when floats are cut to ints
            if (std::find(seenCoords.begin(), seenCoords.end(), currentXYCoord) != seenCoords.end()) continue;

            //Log(std::format("Circle Y: {} -> {}", std::to_string(0), std::to_string(bufferPos.m_Y - centerPos.m_Y)));
            Array2DPosition bufferPosRowCol = {};
            for (int circleY = currentXYCoord.m_Y; circleY >= centerCartesianPos.m_Y - (currentXYCoord.m_Y - centerCartesianPos.m_Y); circleY--)
            {
                bufferPosRowCol = Array2DPosition(circleY, currentXYCoord.m_X);
                seenCoords.push_back(Conversions::ArrayToGrid(bufferPosRowCol));

                //We still want to calculate color for this pos even if it is not valid (since it may be valid in another location
                //so we can add it to the light map
                newColor = CalculateNewColor(data, entity, buffer, Conversions::ArrayToGrid(bufferPosRowCol), centerCartesianPos, &lightLevel, &lightMapChar);
                if (STORE_LIGHT_MAP) data.m_LightMap.push_back(lightMapChar);

                if (!buffer.IsValidPos(bufferPosRowCol)) continue;
                if (buffer.GetAt(bufferPosRowCol)->m_Char == EMPTY_CHAR_PLACEHOLDER) continue;

                //Log(std::format("New color for {} from pos {} from color: {} is: {}", setPos.ToString(), centerPos.ToString(),
                //RaylibUtils::ToString(m_outputBuffer.GetAt(setPos)->m_Color), RaylibUtils::ToString(CalculateNewColor(setPos, centerPos))));


                //We need to have both coords uisng same system so we convert buffer set pos from row col to match XY of center pos
                buffer.SetAt(bufferPosRowCol, newColor);

                if (displayLightLevels)
                {
                    prevLevel = Utils::TryParse<int>(std::string(1, buffer.GetAt(bufferPosRowCol)->m_Char));
                    int newLevel = prevLevel != std::nullopt ? prevLevel.value() + lightLevel : lightLevel;
                    newLevel = static_cast<double>(newLevel) / MAX_LIGHT_LEVEL * 10;
                    lightLevelStr = std::to_string(newLevel)[0];
                    //std::cout << "New level is:{} " << prevLevel.value_or(0) + lightLevel << std::endl;
                    buffer.SetAt(bufferPosRowCol, lightLevelStr[0]);
                }

                if (CACHE_LAST_BUFFER)
                {
                    LogWarning("Updating last frame data");
                    data.m_LastFrameData.emplace_back(bufferPosRowCol , TextChar{newColor, buffer.GetAt(bufferPosRowCol)->m_Char});
                }  
            }
        }
        */
    }

    std::uint8_t LightSourceSystem::CalculateLightLevelFromDistance(const LightSourceData& data, const float& distance) const
    {
        //We do radius +1 since we want there to be 0 light when we go PAST the radius
        return data.m_Intensity* std::powf(1 - (distance / (data.m_LightRadius + 1)), data.m_FalloffStrength);
    }

    Color LightSourceSystem::GetColorFromMultiplier(const Color& originalColor, const Color& filterColor, const float& colorMultiplier) const
    {
        return { static_cast<unsigned char>(std::roundf((originalColor.r) * (1 - colorMultiplier) + (filterColor.r) * (colorMultiplier))),
                 static_cast<unsigned char>(std::roundf((originalColor.g) * (1 - colorMultiplier) + (filterColor.g) * (colorMultiplier))),
                 static_cast<unsigned char>(std::roundf((originalColor.b) * (1 - colorMultiplier) + (filterColor.b) * (colorMultiplier))), 255};
    }

    Color LightSourceSystem::ApplyColorFilter(const Color& originalColor, const Utils::Point3D& fractionalColor, const float& colorMultiplier) const
    {
        return { static_cast<unsigned char>(std::roundf((originalColor.r) * (1 - colorMultiplier) + fractionalColor.m_X)),
                 static_cast<unsigned char>(std::roundf((originalColor.g) * (1 - colorMultiplier) + fractionalColor.m_Y)),
                 static_cast<unsigned char>(std::roundf((originalColor.b) * (1 - colorMultiplier) + fractionalColor.m_Z)), 255 };
    }

    Color LightSourceSystem::CalculateNewColor(LightSourceData& data, const ECS::Entity& entity, 
        const TextBufferPosition& bufferPos, const float& distance, std::uint8_t* outLightLevel, LightMapChar* lightMapChar) const
    {
        //Log(std::format("Distance between {} and {} is: {}",
        //currentPos.ToString(), centerPos.ToString(), std::to_string(distanceToCenter)));

        uint8_t lightLevel = CalculateLightLevelFromDistance(data, distance);
       // Log(std::format("Light level for distance: {} is: {}", std::to_string(distanceToCenter), std::to_string(lightLevel)));
        //std::cout << "Light level is: " << std::to_string(lightLevel) << std::endl;
        if (outLightLevel != nullptr) *outLightLevel = lightLevel;

        //We want to figure out the color data even if the pos is not valid in case we might need
        //light data to be stored even on invalid positions (so the light map can be created)
        const Color filterColor = data.m_GradientFilter.GetColorAt(distance / data.m_LightRadius, false);
        const float colorMultiplier = static_cast<float>(lightLevel) / data.m_Intensity;
        //if (lightMapChar != nullptr) *lightMapChar = LightMapChar(centerPos - currentPos, RaylibUtils::GetFractionalColorRGB(filterColor, colorMultiplier), colorMultiplier);

        const Color originalColor = bufferPos.m_Text.m_Color;
        const Color newColor = GetColorFromMultiplier(originalColor, filterColor, colorMultiplier);

        //Log(std::format("Color multuplier for distance: {} (center {} -> {}) light level: {} is: {} new color: {}",
        //std::to_string(distanceToCenter), centerPos.ToString(), currentPos.ToString(),
        //std::to_string(lightLevel), std::to_string(colorMultiplier), RaylibUtils::ToString(originalColor)));
        return newColor;
    }
}
