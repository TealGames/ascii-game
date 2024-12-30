#include <numbers>
#include <vector>
#include <cstdint>
#include <optional>
#include <limits>
#include <cmath>
#include <iostream>
#include "LightSource.hpp"
#include "Component.hpp"
#include "Point2D.hpp"
#include "Point2DInt.hpp"

#include "EntityRenderer.hpp"
#include "Transform.hpp"
#include "TextBuffer.hpp"
#include "HelperFunctions.hpp"
#include "RaylibUtils.hpp"
#include "ColorGradient.hpp"

namespace ECS
{
    static constexpr bool CACHE_LAST_BUFFER = true;

	LightSource::LightSource(const Transform& transform, const EntityRenderer& renderer, 
        const std::vector<TextBuffer*>& buffers, const ColorGradient& filterColor, const int& lightRadius,
        const std::uint8_t& initialLightLevel, const float& falloffValue) :
		Component(), m_renderer(renderer), m_transform(transform), m_outputBuffers(buffers),
        m_gradientFilter(filterColor), m_lightRadius(lightRadius), m_intensity(initialLightLevel), 
        m_falloffStrength(falloffValue), m_lastFrameData{}
	{
	}

	UpdatePriority LightSource::GetUpdatePriority() const
	{
		return { MIN_PRIORITY };
	}

    void LightSource::Start()
    {

    }

    void LightSource::UpdateStart(float deltaTime)
    {
        m_isDirty = false;
        if (!m_transform.HasMovedThisFrame() && !m_lastFrameData.empty())
        {
            for (const auto& buffer : m_outputBuffers)
                buffer->SetAt(m_lastFrameData);
            return;
        }
        
        //TODO: currently light is only applied on the background layer
        //so we should also make it apply it to other layers too
        if (!m_lastFrameData.empty()) m_lastFrameData.clear();
        RenderLight(false);
        m_isDirty = true;
        //std::cout << "Rendering lgiht" << std::endl;
    }

    void LightSource::UpdateEnd(float deltaTime)
    {
        
    }

    //TODO: this probably needs to be optimized
    //TODO: it seems that when going below a certain point the light becomes brighter/stronger
    void LightSource::CreateLightingForPoint(const Utils::Point2DInt& centerPos, TextBuffer& buffer, bool displayLightLevels)
    {
        //std::cout << "Center pos: " << centerPos.ToString() << std::endl;
        const Utils::Point2DInt maxXY = { centerPos.m_X + m_lightRadius, centerPos.m_Y + m_lightRadius };
        const Utils::Point2DInt minXY = { centerPos.m_X - m_lightRadius, centerPos.m_Y - m_lightRadius };

        //We approxiamte how accurate the circle should be by using the total area to figure out how many points we need
        const int positiveSidePoints = std::min(buffer.GetHeight(), buffer.GetWidth()) / m_lightRadius;
        const float pointXValIncrement = 0.5f;

        //We add all the points on circle using X, Y Coords
        float x = minXY.m_X;
        float y = 0;
        std::vector<Utils::Point2D> bottomCirclePoints = {};
        while (x <= maxXY.m_X)
        {
            y = std::sqrt(std::pow(m_lightRadius, 2) - std::pow(x - centerPos.m_X, 2)) + centerPos.m_Y;
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

                /*Utils::Log(std::format("New color for {} from pos {} from color: {} is: {}", setPos.ToString(), centerPos.ToString(),
                    RaylibUtils::ToString(m_outputBuffer.GetAt(setPos)->m_Color), RaylibUtils::ToString(CalculateNewColor(setPos, centerPos))));*/

                newColor = CalculateNewColor(buffer, bufferPosRowCol.GetFlipped(), centerPos, &lightLevel);
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
                    m_lastFrameData.push_back(TextCharPosition{bufferPosRowCol , TextChar{newColor, buffer.GetAt(bufferPosRowCol)->m_Char}});
            }
        }
    }

	void LightSource::RenderLight(bool displayLightLevels)
	{
        std::vector<std::vector<TextChar>> visualData = m_renderer.GetVisualData();
        Utils::Point2DInt centerPos = {};
        std::cout << "REDNER LIGHT" << std::endl;

        for (const auto& buffer : m_outputBuffers)
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
                    centerPos = m_renderer.GetGlobalVisualPos({ r, c }).GetFlipped();
                    if (!buffer->IsValidPos(centerPos.GetFlipped())) continue;

                    CreateLightingForPoint(centerPos, *buffer, displayLightLevels);
                }
            }
        }
	}

    std::uint8_t LightSource::CalculateLightLevelFromDistance(const float& distance) const
    {
        //We do radius +1 since we want there to be 0 light when we go PAST the radius
        return m_intensity * std::powf(1- (distance/(m_lightRadius+1)), m_falloffStrength);
    }

    Color LightSource::CalculateNewColor(const TextBuffer& buffer, const Utils::Point2DInt& currentPos, 
        const Utils::Point2DInt& centerPos, std::uint8_t* outLightLevel) const
    {
        float distanceToCenter = Utils::GetDistance(currentPos, centerPos);
        /*Utils::Log(std::format("Distance between {} and {} is: {}",
            currentPos.ToString(), centerPos.ToString(), std::to_string(distanceToCenter)));*/

        uint8_t lightLevel = CalculateLightLevelFromDistance(distanceToCenter);
       // Utils::Log(std::format("Light level for distance: {} is: {}", std::to_string(distanceToCenter), std::to_string(lightLevel)));
        //std::cout << "Light level is: " << std::to_string(lightLevel) << std::endl;
        if (outLightLevel != nullptr) *outLightLevel = lightLevel;

        if (!buffer.IsValidPos(currentPos.GetFlipped())) return {};
        Color originalColor = buffer.GetAt(currentPos.GetFlipped())->m_Color;
        Color filterColor = m_gradientFilter.GetColorAt(distanceToCenter / m_lightRadius, false);
        
        //float alpha = m_filterColor.a / 255;
        //TODO: known issue is that below a certain row, colors do not show up
        float colorMultiplier = static_cast<float>(lightLevel) / m_intensity;
    
        originalColor.r = std::roundf((originalColor.r) * (1 - colorMultiplier) + (filterColor.r) * (colorMultiplier));
        originalColor.g = std::roundf((originalColor.g) * (1 - colorMultiplier) + (filterColor.g) * (colorMultiplier));
        originalColor.b = std::roundf((originalColor.b) * (1 - colorMultiplier) + (filterColor.b) * (colorMultiplier));
        originalColor.a = 255;

        /*Utils::Log(std::format("Color multuplier for distance: {} (center {} -> {}) light level: {} is: {} new color: {}",
            std::to_string(distanceToCenter), centerPos.ToString(), currentPos.ToString(),
            std::to_string(lightLevel), std::to_string(colorMultiplier), RaylibUtils::ToString(originalColor)));*/
        return originalColor;
    }
}
