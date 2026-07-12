#include "Core/Serialization/FigSerializers.hpp"

namespace Engine::Serialization
{
	ColHDR4 ToColor(const std::string& value)
	{
		ColHDR4 result = {};
		std::string channelStr = "";
		std::uint8_t currentChannelIndex = 0;
		for (const auto& c : value)
		{
			if (c == ' ')
			{
				result[currentChannelIndex] = std::max(FigFormat::ToFloat(channelStr), 0.0f);
				//If the result has no decimal -> it means we were in [0, 255] scale so we adjust to [0, 1]
				if (channelStr.find('.') == std::string::npos)
					result[currentChannelIndex] /= MAX_INT_COLOR_CHANNEL;

				currentChannelIndex++;
				channelStr = "";
			}
			else channelStr += c;
		}
		result[currentChannelIndex] = std::max(FigFormat::ToFloat(channelStr), 0.0f);
		if (channelStr.find('.') == std::string::npos)
			result[currentChannelIndex] /= MAX_INT_COLOR_CHANNEL;

		return result;
	}
}