#include "pch.hpp"
#include "Core/Asset/SpriteAnimationAsset.hpp"
#include "Fig/Fig.hpp"
#include "Utils/Debug.hpp"
#include "Utils/IOHandler.hpp"
#include "Utils/HelperFunctions.hpp"
#include "Core/Visual/VisualDataParser.hpp"
#include "Utils/ToStringFunctions.hpp"

namespace Engine::Animation
{
	const std::array<std::string_view,1> SpriteAnimationAsset::EXTENSIONS = { ".sanim" };

	//TODO: since structure is similar to input profile asset, it is prob best to abstract special symbols from
	//actual property name and create a general parsing for this config "FIG" data format
	static const char* GENERAL_MARKER = "General";
	static const char* FRAMES_MARKER = "Frames";
	//static const char* FRAME_MARKER = "Frame";
	static const std::string TIME_PROPERTY_NAME_START = "T";

	SpriteAnimationAsset::SpriteAnimationAsset(const std::filesystem::path& path) : Asset(path), m_animation()
	{
		ASSET_EXTENSION_CHECK

		UpdateAssetFromFile();
	}

	const SpriteAnimation& SpriteAnimationAsset::GetAnimation() const
	{
		return m_animation;
	}
	SpriteAnimation& SpriteAnimationAsset::GetAnimationMutable()
	{
		return m_animation;
	}

	void SpriteAnimationAsset::UpdateAssetFromFile()
	{
		FigFormat::Fig fig = FigFormat::Fig(GetAbsolutePathCopy(), FigFormat::FigParseFlag::IncludeOverflowLineStartSpaces);
		//Assert(false, std::format("Found fig:{}", fig.ToString()));

		std::vector<FigFormat::FigPropertyRef> figProperties = {};
		fig.GetAllProperties(GENERAL_MARKER, figProperties);
		//Assert(false, std::format("Get all properties: {} first:{}", std::to_string(figProperties.size()), figProperties[0].GetValue()[0]));

		m_animation.m_Name = figProperties[0].GetValue().front();
		m_animation.m_Loop = FigFormat::ToBool(figProperties[1].GetValue().front());
		m_animation.m_SingleLoopLength = FigFormat::ToFloat(figProperties[2].GetValue().front());
		m_animation.m_AnimationSpeed = FigFormat::ToFloat(figProperties[3].GetValue().front());

		figProperties = {};
		fig.GetAllProperties(FRAMES_MARKER, figProperties);
		//Assert(false, std::format("Fig properties: {} has marker:{}", std::to_string(figProperties.size()), std::to_string(fig.HasMarker(GENERAL_MARKER))));

		//std::vector<std::vector<TextChar>> textCharPos = {};
		std::vector<SpriteAnimationFrame> animationFrames = {};
		//Array2DPosition arrPos = { NULL_INDEX, NULL_INDEX };
		float currentTime = 0;

		/*VisualDataPreset visualPreset = { GetGlobalFont(), VisualData::DEFAULT_FONT_SIZE, VisualData::DEFAULT_CHAR_SPACING,
					CharAreaType::Predefined, VisualData::DEFAULT_PREDEFINED_CHAR_AREA, VisualData::DEFAULT_PIVOT };*/
		Rendering::WorldFontProperties fontSettings = Rendering::WorldFontProperties();

		//Assert(false, std::format("Fig data found:{}", fig.ToString()));
		for (size_t i = 0; i < figProperties.size(); i++)
		{
			//Since time should only have one value we do not need to keep searching
			if (figProperties[i].GetKey().substr(0, TIME_PROPERTY_NAME_START.size()) == TIME_PROPERTY_NAME_START)
			{
				if (!Assert(i % 2 == 0, "Tried to read time property in sprite animation asset of key:{} value:{}"
					"but it occured at index:{} which breaks the desired order of TIME, ANIMATION", figProperties[i].GetKey(),
					Utils::ToStringIterable(figProperties[i].GetValue()), std::to_string(i)))
					return;

				if (!Assert(figProperties[i].GetValue().size() == 1, "Tried to read time property in sprite animation asset of key:{} value:{}"
					"but it contains a value with more than one entries!", figProperties[i].GetKey(),
					Utils::ToStringIterable(figProperties[i].GetValue())))
					return;

				currentTime = FigFormat::ToFloat(figProperties[i].GetValue()[0]);
				continue;
			}

			Rendering::VisualData maybeVisualData = Rendering::ParseDefaultVisualData(figProperties[i].GetValue());
			if (!Assert(!maybeVisualData.IsEmpty(), "Tried to convert fig property value of sprite animation asset of key:{} value:{}"
				"into a visual data using parser but it failed!", figProperties[i].GetKey(),
				Utils::ToStringIterable(figProperties[i].GetValue())))
				return;

			animationFrames.emplace_back(currentTime, maybeVisualData);
		}

		//Assert(false, std::format("Animation frames: {}", std::to_string(animationFrames.size())));
		m_animation.m_Frames = animationFrames;
	}

	void SpriteAnimationAsset::SaveToPath(const std::filesystem::path& path)
	{
		//TODO: implement
	}

	std::string SpriteAnimationAsset::ToString() const
	{
		return std::format("[SpriteAnimationAsset Path:{} SpriteAnimation:{}]", GetAbsolutePathCopy().string(), m_animation.ToString());
	}
}
