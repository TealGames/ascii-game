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

#include "Renderer.hpp"
#include "Transform.hpp"
#include "TextBuffer.hpp"
#include "HelperFunctions.hpp"
#include "RaylibUtils.hpp"

namespace ECS
{
	LightSource::LightSource(const Transform& transform, const Renderer& renderer, 
        const std::vector<TextBuffer*>& buffers, const Color& filterColor, const int& lightRadius,
        const std::uint8_t& initialLightLevel, const float& falloffValue) :
		Component(), m_renderer(renderer), m_transform(transform), m_outputBuffers(buffers),
        m_filterColor(filterColor), m_lightRadius(lightRadius), m_intensity(initialLightLevel), m_falloffStrength(falloffValue)
	{
	}

	UpdatePriority LightSource::GetUpdatePriority() const
	{
		return { MIN_PRIORITY };
	}

    void LightSource::Start()
    {

    }

    void LightSource::Update(float deltaTime)
    {
        //TODO: currently light is only applied on the background layer
        //so we should also make it apply it to other layers too
        RenderLight();
        //std::cout << "Rendering lgiht" << std::endl;
    }

	void LightSource::RenderLight(bool displayLightLevels) const
	{
        std::vector<std::vector<TextChar>> visualData = m_renderer.GetVisualData();
        Utils::Point2DInt centerPos = {};

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
                    //std::cout << "Center pos: " << centerPos.ToString() << std::endl;
                    const Utils::Point2DInt maxXY = { centerPos.m_X + m_lightRadius, centerPos.m_Y + m_lightRadius };
                    const Utils::Point2DInt minXY = { centerPos.m_X - m_lightRadius, centerPos.m_Y - m_lightRadius };

                    //We approxiamte how accurate the circle should be by using the total area to figure out how many points we need
                    const int positiveSidePoints = std::min(buffer->m_HEIGHT, buffer->m_WIDTH) / m_lightRadius;
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
                    Utils::Log(std::format("Circle points: {}", Utils::ToStringIterable<std::vector<Utils::Point2D>, Utils::Point2D>(bottomCirclePoints)));

                    //TODO: ideally, this color should not be hardcoded but should apply
                    //a yellow (or other) color filter using math of the current color
                    const Color lightGreen = { 163, 213, 137, 255 };
                    Utils::Point2DInt currentXYCoord = {};
                    std::vector<Utils::Point2DInt> seenCoords = {};
                    std::uint8_t lightLevel = MIN_LIGHT_LEVEL;
                    std::string lightLevelStr = "";
                    std::optional<int> prevLevel = std::nullopt;

                    for (const auto& point : bottomCirclePoints)
                    {
                        if (point.m_X <= centerPos.m_X) currentXYCoord.m_X = std::floor(point.m_X);
                        else currentXYCoord.m_X = std::ceil(point.m_X);
                        if (!buffer->IsValidCol(currentXYCoord.m_X)) continue;

                        currentXYCoord.m_Y = std::ceil(point.m_Y);
                        if (!buffer->IsValidRow(currentXYCoord.m_Y)) continue;

                        if (buffer->GetAt(currentXYCoord.GetFlipped())->m_Char == EMPTY_CHAR_PLACEHOLDER) continue;

                        //We need to check if it does not have them already since we may have a very short range with many points
                        //which could result in potential duplicates when floats are cut to ints
                        if (std::find(seenCoords.begin(), seenCoords.end(), currentXYCoord) != seenCoords.end()) continue;

                        //Utils::Log(std::format("Circle Y: {} -> {}", std::to_string(0), std::to_string(bufferPos.m_Y - centerPos.m_Y)));
                        Utils::Point2DInt bufferPosRowCol = {};
                        //for (int circleY = 0; circleY <= currentXYCoord.m_Y - centerPos.m_Y; circleY++)
                        for (int circleY = currentXYCoord.m_Y; circleY >= centerPos.m_Y - (currentXYCoord.m_Y - centerPos.m_Y); circleY--)
                        {
                            bufferPosRowCol = Utils::Point2DInt(circleY, currentXYCoord.m_X);
                            /*Utils::Log(std::format("New color for {} from pos {} from color: {} is: {}", setPos.ToString(), centerPos.ToString(),
                                RaylibUtils::ToString(m_outputBuffer.GetAt(setPos)->m_Color), RaylibUtils::ToString(CalculateNewColor(setPos, centerPos))));*/

                                //We need to have both coords uisng same system so we convert buffer set pos from row col to match XY of center pos
                            buffer->SetAt(bufferPosRowCol, CalculateNewColor(*buffer, bufferPosRowCol.GetFlipped(), centerPos, &lightLevel));
                            if (displayLightLevels)
                            {
                                prevLevel = Utils::TryParse<int>(std::string(1, buffer->GetAt(bufferPosRowCol)->m_Char));
                                lightLevelStr = std::string(1, prevLevel != std::nullopt ? prevLevel.value() + lightLevel : lightLevel)[0];
                                buffer->SetAt(bufferPosRowCol, lightLevelStr[0]);
                            }
                            seenCoords.push_back(bufferPosRowCol.GetFlipped());

                            //TODO: maybe to simplify, add the negative versions to the search to prevent duplicate code

                            ////Utils::Log(std::format("Setting pos: {} to lgiht green", Utils::Point2DInt({ centerPos.m_Y + circleY, bufferPos.m_X }).ToString()));
                            //if (circleY != 0)
                            //{
                            //    bufferPosRowCol = Utils::Point2DInt(centerPos.m_Y - circleY, currentXYCoord.m_X);
                            //    m_outputBuffer.SetAt(bufferPosRowCol, CalculateNewColor(bufferPosRowCol.GetFlipped(), centerPos, &lightLevel));
                            //    //m_outputBuffer.SetAt(setPos, lightGreen);

                            //    if (displayLightLevels)
                            //    {
                            //        prevLevel = Utils::TryParse<int>(std::string(1, m_outputBuffer.GetAt(bufferPosRowCol)->m_Char));
                            //        lightLevelStr = std::string(1, prevLevel != std::nullopt ? prevLevel.value() + lightLevel : lightLevel)[0];
                            //        m_outputBuffer.SetAt(bufferPosRowCol, lightLevelStr[0]);
                            //    }
                            //}
                        }
                       
                    }
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
        
        //float alpha = m_filterColor.a / 255;
        float colorMultiplier = static_cast<float>(lightLevel) / m_intensity;
    
        originalColor.r = std::roundf((originalColor.r) * (1 - colorMultiplier) + (m_filterColor.r) * (colorMultiplier));
        originalColor.g = std::roundf((originalColor.g) * (1 - colorMultiplier) + (m_filterColor.g) * (colorMultiplier));
        originalColor.b = std::roundf((originalColor.b) * (1 - colorMultiplier) + (m_filterColor.b) * (colorMultiplier));

        Utils::Log(std::format("Color multuplier for distance: {} (center {} -> {}) light level: {} is: {} new color: {}",
            std::to_string(distanceToCenter), centerPos.ToString(), currentPos.ToString(),
            std::to_string(lightLevel), std::to_string(colorMultiplier), RaylibUtils::ToString(originalColor)));
        return originalColor;
    }
}
