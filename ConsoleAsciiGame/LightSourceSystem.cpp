#include <numbers>
#include <limits>
#include <cmath>

#include "pch.hpp"
#include "LightSourceSystem.hpp"
#include "Component.hpp"
#include "Point2D.hpp"
#include "Point2DInt.hpp"
#include "Scene.hpp"
#include "EntityRendererSystem.hpp"
#include "TransformSystem.hpp"
#include "TextBuffer.hpp"
#include "HelperFunctions.hpp"
#include "RaylibUtils.hpp"
#include "ColorGradient.hpp"

namespace ECS
{
    static constexpr bool CACHE_LAST_BUFFER = true;

	LightSourceSystem::LightSourceSystem(const TransformSystem& transform, const EntityRendererSystem& renderer) :
        m_transformSystem(transform), m_rendererSystem(renderer)
	{
	}

    void LightSourceSystem::SystemUpdate(Scene& scene, const float& deltaTime)
    {
        std::vector<TextBuffer*> affectedLayerBuffers = {};
        scene.OperateOnComponents<LightSourceData>(
            [this, &scene, &affectedLayerBuffers](LightSourceData& data, ECS::Entity& entity)-> void
            {
                data.m_Dirty = false;

                affectedLayerBuffers = scene.GetTextBuffersMutable(data.m_AffectedLayers);
                if (!m_transformSystem.HasMovedThisFrame(entity.m_Transform) 
                    && !data.m_LastFrameData.empty())
                {
                    for (const auto& buffer : affectedLayerBuffers)
                        buffer->SetAt(data.m_LastFrameData);
                    return;
                }

                if (!data.m_LastFrameData.empty()) data.m_LastFrameData.clear();
                RenderLight(data, entity, affectedLayerBuffers);
                data.m_Dirty = true;
                //std::cout << "Rendering lgiht" << std::endl;
            });
    }

    
    //TODO: this probably needs to be optimized
    //TODO: it seems that when going below a certain point the light becomes brighter/stronger
    void LightSourceSystem::CreateLightingForPoint(LightSourceData& data, const ECS::Entity& entity, 
        const Utils::Point2DInt& centerPos, TextBuffer& buffer, bool displayLightLevels)
    {
        
        //std::cout << "Center pos: " << centerPos.ToString() << std::endl;
        int radius = data.m_LightRadius;
        const Utils::Point2DInt maxXY = { centerPos.m_X + radius, centerPos.m_Y + radius };
        const Utils::Point2DInt minXY = { centerPos.m_X - radius, centerPos.m_Y - radius };

        //We approxiamte how accurate the circle should be by using the total area to figure out how many points we need
        const int positiveSidePoints = std::min(buffer.GetHeight(), buffer.GetWidth()) / radius;
        const float pointXValIncrement = 0.5f;

        //We add all the points on circle using X, Y Coords
        float x = minXY.m_X;
        float y = 0;
        std::vector<Utils::Point2D> bottomCirclePoints = {};
        while (x <= maxXY.m_X)
        {
            y = std::sqrt(std::pow(data.m_LightRadius, 2) - std::pow(x - centerPos.m_X, 2)) + centerPos.m_Y;
            bottomCirclePoints.push_back({ x, y });

            x += pointXValIncrement;
        }
        //Utils::Log(std::format("Circle points: {}", Utils::ToStringIterable<std::vector<Utils::Point2D>, Utils::Point2D>(bottomCirclePoints)));

        Color newColor = {};
        Utils::Point2DInt currentXYCoord = {};
        std::vector<Utils::Point2DInt> seenCoords = {};
        std::uint8_t lightLevel = MIN_LIGHT_LEVEL;
        std::string lightLevelStr = "";
        std::optional<int> prevLevel = std::nullopt;
        bool isInitialPosValid = false;

        for (const auto& point : bottomCirclePoints)
        {
            if (point.m_X <= centerPos.m_X) currentXYCoord.m_X = std::floor(point.m_X);
            else currentXYCoord.m_X = std::ceil(point.m_X);

            currentXYCoord.m_Y = std::ceil(point.m_Y);
            isInitialPosValid = buffer.IsValidPos(currentXYCoord.GetFlipped());

            //We need to check if it does not have them already since we may have a very short range with many points
            //which could result in potential duplicates when floats are cut to ints
            if (std::find(seenCoords.begin(), seenCoords.end(), currentXYCoord) != seenCoords.end()) continue;

            //Utils::Log(std::format("Circle Y: {} -> {}", std::to_string(0), std::to_string(bufferPos.m_Y - centerPos.m_Y)));
            Utils::Point2DInt bufferPosRowCol = {};
            for (int circleY = currentXYCoord.m_Y; circleY >= centerPos.m_Y - (currentXYCoord.m_Y - centerPos.m_Y); circleY--)
            {
                bufferPosRowCol = Utils::Point2DInt(circleY, currentXYCoord.m_X);
                seenCoords.push_back(bufferPosRowCol.GetFlipped());

                if (!buffer.IsValidPos(bufferPosRowCol)) continue;
                if (buffer.GetAt(bufferPosRowCol)->m_Char == EMPTY_CHAR_PLACEHOLDER) continue;

                //Utils::Log(std::format("New color for {} from pos {} from color: {} is: {}", setPos.ToString(), centerPos.ToString(),
                //RaylibUtils::ToString(m_outputBuffer.GetAt(setPos)->m_Color), RaylibUtils::ToString(CalculateNewColor(setPos, centerPos))));

                newColor = CalculateNewColor(data, entity, buffer, bufferPosRowCol.GetFlipped(), centerPos, &lightLevel);
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
                    data.m_LastFrameData.push_back(TextCharPosition{ bufferPosRowCol , TextChar{newColor, buffer.GetAt(bufferPosRowCol)->m_Char} });
            }
        }
    }

	void LightSourceSystem::RenderLight(LightSourceData& data, ECS::Entity& entity, 
        std::vector<TextBuffer*>& buffers, bool displayLightLevels)
	{
        EntityRendererData* renderData = entity.TryGetComponent<EntityRendererData>();
        if (!Utils::Assert(renderData != nullptr, std::format("Tried to render light for entity: {} "
            "but could not find its entity render component!", entity.m_Name))) return;

        //TODO: it might not make sense for all lighting to just use the renderer to determine lighting start pos,
        //so perhaps this needs to be more customizable to allow for this and other behavior
        std::vector<std::vector<TextChar>> visualData = renderData->m_VisualData;
        Utils::Point2DInt centerPos = {};
       // std::cout << "REDNER LIGHT" << std::endl;

        for (const auto& buffer : buffers)
        {
            if (buffer == nullptr) continue;
            //Utils::Log(std::format("When rendering light start colors: {}", buffer->ToString(false)));
            //Utils::Log(std::format("Player Pos color: {}", RaylibUtils::ToString(buffer->GetAt(m_transform.m_Pos.GetFlipped())->m_Color)));

            for (int r = 0; r < visualData.size(); r++)
            {
                for (int c = 0; c < visualData[r].size(); c++)
                {
                    //we use default coords (x, y) but visual pos is in (row, col) so we flip
                    //TODO: maybe abstract transform from one coord system to the other
                    centerPos = m_rendererSystem.GetGlobalVisualPos({ r, c }, *renderData, entity).GetFlipped();
                    if (!buffer->IsValidPos(centerPos.GetFlipped())) continue;

                    CreateLightingForPoint(data, entity, centerPos, *buffer, displayLightLevels);
                }
            }
        }
	}

    std::uint8_t LightSourceSystem::CalculateLightLevelFromDistance(const LightSourceData& data, const float& distance) const
    {
        //We do radius +1 since we want there to be 0 light when we go PAST the radius
        return data.m_Intensity* std::powf(1 - (distance / (data.m_LightRadius + 1)), data.m_FalloffStrength);
    }

    Color LightSourceSystem::CalculateNewColor(LightSourceData& data, const ECS::Entity& entity, 
        const TextBuffer& buffer, const Utils::Point2DInt& currentPos,
        const Utils::Point2DInt& centerPos, std::uint8_t* outLightLevel) const
    {
        float distanceToCenter = Utils::GetDistance(currentPos, centerPos);
        //Utils::Log(std::format("Distance between {} and {} is: {}",
        //currentPos.ToString(), centerPos.ToString(), std::to_string(distanceToCenter)));

        uint8_t lightLevel = CalculateLightLevelFromDistance(data, distanceToCenter);
       // Utils::Log(std::format("Light level for distance: {} is: {}", std::to_string(distanceToCenter), std::to_string(lightLevel)));
        //std::cout << "Light level is: " << std::to_string(lightLevel) << std::endl;
        if (outLightLevel != nullptr) *outLightLevel = lightLevel;

        if (!buffer.IsValidPos(currentPos.GetFlipped())) return {};
        Color originalColor = buffer.GetAt(currentPos.GetFlipped())->m_Color;
        Color filterColor = data.m_GradientFilter.GetColorAt(distanceToCenter / data.m_LightRadius, false);
        
        //float alpha = m_filterColor.a / 255;
        //TODO: known issue is that below a certain row, colors do not show up
        float colorMultiplier = static_cast<float>(lightLevel) / data.m_Intensity;
    
        originalColor.r = std::roundf((originalColor.r) * (1 - colorMultiplier) + (filterColor.r) * (colorMultiplier));
        originalColor.g = std::roundf((originalColor.g) * (1 - colorMultiplier) + (filterColor.g) * (colorMultiplier));
        originalColor.b = std::roundf((originalColor.b) * (1 - colorMultiplier) + (filterColor.b) * (colorMultiplier));
        originalColor.a = 255;

        //Utils::Log(std::format("Color multuplier for distance: {} (center {} -> {}) light level: {} is: {} new color: {}",
        //std::to_string(distanceToCenter), centerPos.ToString(), currentPos.ToString(),
        //std::to_string(lightLevel), std::to_string(colorMultiplier), RaylibUtils::ToString(originalColor)));
        return originalColor;
    }
}
