#include "pch.hpp"
#include "Fig/FigDeserializers.hpp"
#include "Utils/StringUtil.hpp"
#include "Utils/HelperFunctions.hpp"

bool ToBool(const std::string& value)
{
	return Utils::StringUtil(value).ToLowerCase().ToString() == "true";
}

int ToInt32(const std::string& value)
{
	return Utils::TryParse<int>(value).value_or(0);
}
float ToFloat(const std::string& value)
{
	return Utils::TryParse<float>(value).value_or(0);
}
std::uint8_t ToUint8(const std::string& value)
{
	return Utils::TryParse<std::uint8_t>(value).value_or(0);
}
Color ToColor(const std::string& value)
{
	Color result = {};
	std::string channelStr = "";
	std::uint8_t currentChannelIndex = 0;
	for (const auto& c : value)
	{
		if (c == ' ')
		{
			result[currentChannelIndex] = std::max(ToFloat(channelStr), 0.0f);
			//If the result has no decimal -> it means we were in [0, 255] scale so we adjust to [0, 1]
			if (channelStr.find('.') == std::string::npos) 
				result[currentChannelIndex] /= MAX_INT_COLOR_CHANNEL;

			currentChannelIndex++;
			channelStr = "";
		}
		else channelStr += c;
	}
	result[currentChannelIndex] = std::max(ToFloat(channelStr), 0.0f);
	if (channelStr.find('.') == std::string::npos)
		result[currentChannelIndex] /= MAX_INT_COLOR_CHANNEL;

	return result;
}