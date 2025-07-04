#include "pch.hpp"
#include "Core/Asset/SpriteAnimationAsset.hpp"
#include "Fig/Fig.hpp"
#include "Fig/FigDeserializers.hpp"
#include "Core/Analyzation/Debug.hpp"
#include "Utils/IOHandler.hpp"
#include "Utils/HelperFunctions.hpp"
#include "Core/Visual/VisualDataParser.hpp"
#include "StaticReferenceGlobals.hpp"

const std::string SpriteAnimationAsset::EXTENSION = ".sanim";

//TODO: since structure is similar to input profile asset, it is prob best to abstract special symbols from
//actual property name and create a general parsing for this config "FIG" data format
static const char* GENERAL_MARKER = "General";
static const char* FRAMES_MARKER = "Frames";
//static const char* FRAME_MARKER = "Frame";
static const std::string TIME_PROPERTY_NAME_START = "T";

SpriteAnimationAsset::SpriteAnimationAsset(const std::filesystem::path& path) : Asset(path, false), m_animation()
{
	if (!Assert(IO::DoesPathHaveExtension(path, EXTENSION), std::format("Tried to create a sprite animation asset from path:'{}' "
		"but it does not have required extension:'{}'", path.string(), EXTENSION)))
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
	Fig fig = Fig(GetPathCopy(), FigFlag::IncludeOverflowLineStartSpaces);
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

	//std::vector<std::vector<TextChar>> textCharPos = {};
	std::vector<SpriteAnimationFrame> animationFrames = {};
	//Array2DPosition arrPos = { NULL_INDEX, NULL_INDEX };
	float currentTime = 0;

	/*VisualDataPreset visualPreset = { GetGlobalFont(), VisualData::DEFAULT_FONT_SIZE, VisualData::DEFAULT_CHAR_SPACING,
				CharAreaType::Predefined, VisualData::DEFAULT_PREDEFINED_CHAR_AREA, VisualData::DEFAULT_PIVOT };*/
	FontProperties fontSettings = FontProperties(VisualData::DEFAULT_FONT_SIZE, 0, StaticReferenceGlobals::GetDefaultRaylibFont());

	//Assert(false, std::format("Fig data found:{}", fig.ToString()));
	for (size_t i=0; i< figProperties.size(); i++)
	{
		//Since time should only have one value we do not need to keep searching
		if (figProperties[i].GetKey().substr(0, TIME_PROPERTY_NAME_START.size()) == TIME_PROPERTY_NAME_START)
		{
			if (!Assert(i % 2 == 0, std::format("Tried to read time property in sprite animation asset of key:{} value:{}"
				"but it occured at index:{} which breaks the desired order of TIME, ANIMATION", figProperties[i].GetKey(), 
				Utils::ToStringIterable<std::vector<std::string>, std::string>(figProperties[i].GetValue()), std::to_string(i))))
				return;

			if (!Assert(figProperties[i].GetValue().size() == 1, std::format("Tried to read time property in sprite animation asset of key:{} value:{}"
				"but it contains a value with more than one entries!", figProperties[i].GetKey(),
				Utils::ToStringIterable<std::vector<std::string>, std::string>(figProperties[i].GetValue()))))
				return;

			currentTime = ToFloat(figProperties[i].GetValue()[0]);
			continue;
		}

		VisualData maybeVisualData = ParseDefaultVisualData(figProperties[i].GetValue());
		if (!Assert(!maybeVisualData.IsEmpty(), std::format("Tried to convert fig property value of sprite animation asset of key:{} value:{}"
			"into a visual data using parser but it failed!", figProperties[i].GetKey(),
			Utils::ToStringIterable<std::vector<std::string>, std::string>(figProperties[i].GetValue()))))
			return;

		//LogError(std::format("Began creating new frame with time:{} visual:{}", std::to_string(currentTime), visualData.ToString()));

		//SpriteAnimationFrame newFrame = SpriteAnimationFrame();
		//LogError(std::format("Trying to add sprite anim frame:{}", newFrame.ToString()));

		animationFrames.emplace_back(currentTime, maybeVisualData);
		//LogError(std::format("created visual data:{}", maybeVisualData.ToString()));
		//textCharPos.clear();
		//arrPos = { NULL_INDEX, NULL_INDEX };
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
	return std::format("[SpriteAnimationAsset Path:{} SpriteAnimation:{}]", GetPathCopy().string(), m_animation.ToString());
}