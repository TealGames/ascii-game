#include "pch.hpp"
#include "RenderLayer.hpp"
#include "TextBuffer.hpp"
#include "HelperFunctions.hpp"
#include "Debug.hpp"

std::string ToString(const RenderLayerType& layers)
{
	return Utils::ToStringIterable<std::vector<std::string>, std::string>(GetLayersAsStrings(layers));
}
std::vector<std::string> GetLayersAsStrings(const RenderLayerType& layers)
{
	std::vector<std::string> stringLayers = {};
	if (Utils::HasFlagAll<RenderLayerType>(layers, RenderLayerType::Background)) stringLayers.push_back("Background");
	if (Utils::HasFlagAll<RenderLayerType>(layers, RenderLayerType::Player)) stringLayers.push_back("Player");
	if (Utils::HasFlagAll<RenderLayerType>(layers, RenderLayerType::UI)) stringLayers.push_back("UI");
	return stringLayers;
}
RenderLayerType GetLayersFromStrings(const std::vector<std::string> layerStrs)
{
	//Assert(false, std::format("Trying to get layers from strings; {}", Utils::ToStringIterable<std::vector<std::string>, std::string>(layerStrs)));
	RenderLayerType layers = RenderLayerType::None;
	for (const auto& layerStr : layerStrs)
	{
		if (layerStr == "Background") layers |= RenderLayerType::Background;
		else if (layerStr == "Player")
		{
			layers |= RenderLayerType::Player;
			//Assert(false, std::format("Adding player layer ZTO stirng yeah. result: {}", ToString(layers)));
		}
		else if (layerStr == "UI") layers |= RenderLayerType::UI;
		else
		{
			Assert(false, std::format("Tried to get layers from strings: {} but encountered layer name that could not be parsed: '{}'",
				Utils::ToStringIterable < std::vector<std::string>, std::string>(layerStrs), layerStr));
		}
	}
	return layers;
}

RenderLayerType operator|(const RenderLayerType& lhs, const RenderLayerType& rhs)
{
	return static_cast<RenderLayerType>(
		static_cast<RenderLayerNumericType>(lhs) | static_cast<RenderLayerNumericType>(rhs));
}

RenderLayerType& operator|=(RenderLayerType& lhs, const RenderLayerType& rhs)
{
	lhs = static_cast<RenderLayerType>(lhs | rhs);
	return lhs;
}

RenderLayerType operator&(const RenderLayerType& lhs, const RenderLayerType& rhs)
{
	return static_cast<RenderLayerType>(
		static_cast<RenderLayerNumericType>(lhs) & static_cast<RenderLayerNumericType>(rhs));
}

RenderLayerType& operator&=(RenderLayerType& lhs, const RenderLayerType& rhs)
{
	lhs = static_cast<RenderLayerType>(lhs & rhs);
	return lhs;
}

bool operator==(const RenderLayerType& lhs, const RenderLayerNumericType& rhs)
{
	return static_cast<RenderLayerNumericType>(lhs) == rhs;
}

bool operator!=(const RenderLayerType& lhs, const RenderLayerNumericType& rhs)
{
	return static_cast<RenderLayerNumericType>(lhs) != rhs;
}

RenderLayer::RenderLayer() : m_buffer{} {}
RenderLayer::RenderLayer(const FragmentedTextBuffer& buffer) : m_buffer(buffer) {}

void RenderLayer::AddText(const TextBufferCharPosition& bufferPos)
{
	m_buffer.push_back(bufferPos);
}

//WorldTextureBuffer& RenderLayer::GetBufferMutable()
//{
//	return m_buffer;
//}

FragmentedTextBuffer& RenderLayer::GetBufferMutable()
{
	return m_buffer;
}

const FragmentedTextBuffer& RenderLayer::GetBuffer() const
{
	return m_buffer;
}

//RenderLayer::RenderLayer(const RawTextBufferBlock& rawBuffer, const int& fontSize, 
//	const Utils::Point2DInt& charSpacing) :
//	m_fontSize(fontSize), m_CharSpacing(charSpacing), m_rawTextBuffer(rawBuffer),
//	m_SquaredTextBuffer(),
//	m_defaultSquaredTextBuffer(m_SquaredTextBuffer)
//{
//	/*Log(std::format("BEFORE Squared buffer: ({}x{}) {}",
//		std::to_string(m_SquaredTextBuffer.GetWidth()),
//		std::to_string(m_SquaredTextBuffer.GetHeight()),
//		m_SquaredTextBuffer.ToString()));*/
//
//	if (rawBuffer.empty()) return;
//
//	std::optional<TextBuffer> squaredBuffer = CreateSquaredBuffer();
//	if (!Assert(squaredBuffer.has_value(), std::format("Tried to create a render layer with raw buffer: {}, "
//		"but failed to square data", ToStringRawBuffer(m_rawTextBuffer)))) return;
//	//Log(std::format("Square buffer val in optional: {}", squaredBuffer.value().ToString()));
//
//	m_SquaredTextBuffer = squaredBuffer.value();
//	m_defaultSquaredTextBuffer = m_SquaredTextBuffer;
//	
//	/*Log(std::format("AFTER Squared buffer: ({}x{}) {}",
//		std::to_string(m_SquaredTextBuffer.GetWidth()),
//		std::to_string(m_SquaredTextBuffer.GetHeight()),
//		m_SquaredTextBuffer.ToString()));*/
//
//	/*Log(std::format("USING RAW CONSTURCUTOR Creating square buffer: {} default square; {}",
//		m_SquaredTextBuffer.ToString(), m_defaultSquaredTextBuffer.ToString()));*/
//}

//RenderLayer::RenderLayer(const TextBuffer& squareBuffer, const int& fontSize,
//	const Utils::Point2DInt& charSpacing) :
//	m_fontSize(fontSize), m_CharSpacing(charSpacing), m_rawTextBuffer(),
//	m_SquaredTextBuffer(squareBuffer), m_defaultSquaredTextBuffer(m_SquaredTextBuffer)
//{
//	//We need to copy inside becuase if the text buffer had its data popualted in the constructor body, then
//	//the copy in the initializer list will only copy data from the square's initialzier list member values, not after
//	//the full body constructor runs, so this guarantees we copy it after it is run
//	m_defaultSquaredTextBuffer = m_SquaredTextBuffer;
//	/*Log(std::format("USING SQUAER CONSTURCUTOR Creating square buffer: {} default square; {}", 
//		m_SquaredTextBuffer.ToString(), m_defaultSquaredTextBuffer.ToString()));*/
//}

//std::optional<TextBuffer> RenderLayer::CreateSquaredBuffer() const
//{
//	//Log("Began creating squared buffer");
//	if (m_rawTextBuffer.empty()) return TextBuffer{};
//
//	std::vector<std::vector<TextChar>> filledSpaces = {};
//	filledSpaces.reserve(m_rawTextBuffer.size());
//
//	int maxRowCharLen = 0;
//	int adjacentColDiff = 0;
//
//	for (const auto& row : m_rawTextBuffer)
//	{
//		if (row.size() > maxRowCharLen) maxRowCharLen = row.size();
//
//		filledSpaces.push_back({});
//		filledSpaces.back().reserve(maxRowCharLen);
//
//		for (int i = 1; i < row.size(); i++)
//		{
//			filledSpaces.back().push_back(row[i - 1].m_Text);
//			adjacentColDiff = row[i].m_RowColPos.GetRow() - row[i - 1].m_RowColPos.GetCol();
//
//			if (adjacentColDiff > 1)
//			{
//				for (int i = 0; i < adjacentColDiff - 1; i++)
//				{
//					filledSpaces.back().push_back(TextChar(Color(), EMPTY_CHAR_PLACEHOLDER));
//				}
//			}
//		}
//		filledSpaces.back().push_back(row.back().m_Text);
//	}
//	//Log(std::format("After intercolumn check max len: {} actual chars; {}", std::to_string(maxRowCharLen), TextBuffer::ToString(filledSpaces)));
//
//	for (int r = 0; r < filledSpaces.size(); r++)
//	{
//		if (filledSpaces[r].size() == maxRowCharLen) continue;
//		/*if (!Assert(filledSpaces[r].size() < maxRowCharLen,
//			std::format("Tried to get rectangalized scene text for render layer but the current "
//				"row size: {} is bigger than the max: {}",
//				std::to_string(filledSpaces[r].size()), std::to_string(maxRowCharLen))))
//			return std::nullopt;*/
//
//		for (int i = 0; i < filledSpaces[r].size() - maxRowCharLen; i++)
//			filledSpaces[r].push_back(TextChar(Color(), EMPTY_CHAR_PLACEHOLDER));
//	}
//	TextBuffer result= TextBuffer(filledSpaces[0].size(), filledSpaces.size(), filledSpaces);
//	/*Log(std::format("Size: {}x{} result: {}", 
//		std::to_string(filledSpaces.size()), std::to_string(filledSpaces[0].size()), result.ToString()));*/
//	return result;
//}

void RenderLayer::ResetToDefault()
{
	//Log("Resetting to default: "+m_SquaredTextBuffer.ToString());
	m_buffer.clear();
}

//std::string RenderLayer::ToStringRawBuffer(const RawTextBufferBlock& block)
//{
//	std::string result = "\nRaw Buffer:\n";
//	for (const auto& row : block)
//	{
//		for (const auto& charPos : row)
//		{
//			result += charPos.ToString() + " ";
//		}
//		result += "\n";
//	}
//	return result;
//}

std::string RenderLayer::ToString() const
{
	std::vector<std::string> layerStr = {};
	for (const auto& text : m_buffer)
	{
		layerStr.push_back(text.ToString());
	}
	return Utils::ToStringIterable<std::vector<std::string>, std::string>(layerStr);
}