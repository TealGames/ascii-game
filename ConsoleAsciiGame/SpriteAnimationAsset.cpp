#include "pch.hpp"
#include "SpriteAnimationAsset.hpp"
#include "Fig.hpp"
#include "FigDeserializers.hpp"
#include "Debug.hpp"
#include "IOHandler.hpp"

const std::string SpriteAnimationAsset::EXTENSION = ".sanim";

//TODO: since structure is similar to input profile asset, it is prob best to abstract special symbols from
//actual property name and create a general parsing for this config "FIG" data format
static const char* GENERAL_MARKER = "General";
static const char* FRAMES_MARKER = "Frames";
//static const char* FRAME_MARKER = "Frame";
static const std::string TIME_PROPERTY_NAME_START = "T";
static constexpr char NEW_ROW_SYMBOL = '-';

SpriteAnimationAsset::SpriteAnimationAsset(const std::filesystem::path& path) : Asset(path, false), m_animation()
{
	if (!Assert(this, IO::DoesPathHaveExtension(path, EXTENSION), std::format("Tried to create a sprite animation asset from path:'{}' "
		"but it does not have required extension:'{}'", EXTENSION)))
		return;

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
	Fig fig = Fig(GetPath());
	//Assert(false, std::format("Found fig:{}", fig.ToString()));

	std::vector<FigPropertyRef> figProperties = {};
	fig.GetAllProperties(GENERAL_MARKER, figProperties);
	//Assert(false, std::format("Get all properties: {} first:{}", std::to_string(figProperties.size()), figProperties[0].GetValue()[0]));

	m_animation.m_Name = figProperties[0].GetValue().front();
	m_animation.m_Loop = ToBool(figProperties[1].GetValue().front());
	m_animation.m_SingleLoopLength = ToFloat(figProperties[2].GetValue().front());
	m_animation.m_AnimationSpeed = ToFloat(figProperties[3].GetValue().front());

	figProperties = {};
	fig.GetAllProperties(FRAMES_MARKER, figProperties);
	//Assert(false, std::format("Fig properties: {} has marker:{}", std::to_string(figProperties.size()), std::to_string(fig.HasMarker(GENERAL_MARKER))));

	std::vector<std::vector<TextCharArrayPosition>> textCharPos = {};
	std::vector<SpriteAnimationFrame> animationFrames = {};
	Array2DPosition arrPos = { NULL_INDEX, NULL_INDEX };
	float currentTime = 0;

	VisualDataPreset visualPreset = { GetGlobalFont(), VisualData::DEFAULT_FONT_SIZE, VisualData::DEFAULT_CHAR_SPACING,
				CharAreaType::Predefined, VisualData::DEFAULT_PREDEFINED_CHAR_AREA, VisualData::DEFAULT_PIVOT };

	//Assert(false, std::format("Fig data found:{}", fig.ToString()));
	for (size_t i=0; i< figProperties.size(); i++)
	{
		LogError(std::format("Iteraing on proeprty:{} at:{}", figProperties[i].GetKey(), std::to_string(i)));
		for (const auto& value : figProperties[i].GetValue())
		{
			//Since time should only have one value we do not need to keep searching
			if (figProperties[i].GetKey().substr(0, TIME_PROPERTY_NAME_START.size()) == TIME_PROPERTY_NAME_START)
			{
				if (!Assert(this, i % 2 == 0, std::format("Tried to read time property in sprite animation asset of key:{} value:{}"
					"but it occured at index:{} which breaks the desired order", figProperties[i].GetKey(), value, std::to_string(i))))
					return;

				currentTime = ToFloat(value);
				break;
			}
				
			for (size_t j=0; j<value.size(); j++)
			{
				LogError(std::format("Iteraing on proeprty:{} at:{} value:{} char:{}", figProperties[i].GetKey(), std::to_string(i), value, Utils::ToString(value[j])));
				if (j==0)
				{
					if (!Assert(this, value[j]==NEW_ROW_SYMBOL, std::format("Tried to update sprite animation asset from file "
						"but at frame property:{} value:{} the first char:{} is not the ROW SYMBOL CHAR:{}", figProperties[i].ToString(), 
						value, Utils::ToString(value[0]), Utils::ToString(NEW_ROW_SYMBOL))))
						return;

					arrPos.IncrementRow(1);
					arrPos.SetCol(0);
					textCharPos.push_back(std::vector<TextCharArrayPosition>{});
					continue;
				}

				//TODO: change the color to use the color assigned in the animator
				textCharPos.back().push_back(TextCharArrayPosition(arrPos, TextChar(WHITE, value[j])));
				arrPos.IncrementCol(1);
			}
		}

		if (i % 2 == 1)
		{
			VisualData visualData = VisualData(textCharPos, visualPreset);
			LogError(std::format("Began creating new frame with time:{} visual:{}", std::to_string(currentTime), visualData.ToString()));

			SpriteAnimationFrame newFrame = SpriteAnimationFrame(currentTime, visualData);
			//LogError(this, std::format("Trying to add sprite anim frame:{}", newFrame.ToString()));

			animationFrames.push_back(newFrame);
			textCharPos.clear();
			arrPos = { NULL_INDEX, NULL_INDEX };
		}
	}

	//Assert(false, std::format("Animation frames: {}", std::to_string(animationFrames.size())));
	m_animation.SetVisualsFromFrames(animationFrames);
}

void SpriteAnimationAsset::SaveToPath(const std::filesystem::path& path)
{
	//TODO: implement
}

std::string SpriteAnimationAsset::ToString() const
{
	return std::format("[SpriteAnimationAsset Path:{} SpriteAnimation:{}]", GetPath().string(), m_animation.ToString());
}