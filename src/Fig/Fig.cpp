#include <fstream>
#include <cctype>
#include "pch.hpp"
#include "Fig/Fig.hpp"
#include "Utils/HelperFunctions.hpp"
#include "Utils/IOHandler.hpp"
#include "Utils/StringUtil.hpp"
#include "Utils/Debug.hpp"

namespace FigFormat
{
	const std::string Fig::COMMENT_START = "/*";
	const std::string Fig::COMMENT_CLOSE = "*\\";

	static constexpr const char* BOOL_VALUE_TRUE = "true";
	static constexpr const char* BOOL_VALUE_FALSE = "false";

	FigPropertyRef::FigPropertyRef(const std::string& key, const FigValue& value)
		: m_Key(key.c_str()), m_Value(&value) {}

	std::string FigPropertyRef::GetKey() const
	{
		return m_Key;
	}
	const FigValue& FigPropertyRef::GetValue() const
	{
		ENGINE_ASSERT(m_Value != nullptr, "Tried to get value from FIG property with key:{} "
			"but value is null", GetKey());

		return *m_Value;
	}

	std::string FigPropertyRef::ToString() const
	{
		return std::format("[Key:{} Value:{}]", GetKey(), ::Utils::ToStringIterable(GetValue()));
	}

	FigProperty::FigProperty(const std::string& key, const FigValue& value)
		: m_Key(key), m_Value(value) {}

	FigProperty::FigProperty() : FigProperty("", {}) {}

	bool FigProperty::IsEmpty() const
	{
		return m_Key.empty() || m_Value.empty();
	}

	std::string FigProperty::ToString() const
	{
		return std::format("[Key:{} Value:{}]", m_Key, ::Utils::ToStringIterable(m_Value));
	}

	Fig::Fig() : m_properties(), m_markedProperties() {}
	Fig::Fig(const std::string& contents) : Fig()
	{
		std::vector<std::string> fileLines = { "" };
		size_t lineIndex = 0;
		for (const char& c : contents)
		{
			if (c == '\n')
			{
				fileLines.emplace_back("");
				lineIndex++;
				continue;
			}
			fileLines[lineIndex].push_back(c);
		}
		CreateFigFormat(fileLines);
	}
	Fig::Fig(const std::filesystem::path& path, const FigParseFlag flag) : Fig()
	{
		std::string currentLine = "";
		std::fstream fstream = std::fstream(path);
		std::vector<std::string> fileLines = {};

		while (std::getline(fstream, currentLine))
		{
			fileLines.emplace_back(currentLine);
		}
		CreateFigFormat(fileLines, flag);
	}
	Fig::~Fig()
	{
		for (auto& markedSection : m_markedProperties)
		{
			delete markedSection.second;
		}
		m_markedProperties = {};
	}

	void Fig::ParseValue(std::vector<std::string>& buffer, const std::string& line)
	{
		if (line.empty()) return;

		//Be default we always want to push a new value since if it is empty -> we want new val
		//if we need a new line -> we want to push new line
		buffer.emplace_back("");
		bool foundSeparator = false;

		//Note: this is not too much worse than string.find since it is a naive search
		for (const auto& c : line)
		{
			if (c == VALUE_SEPARATOR_CHAR)
			{
				foundSeparator = true;
				buffer.emplace_back("");
				continue;
			}
			if (foundSeparator && c == ' ')
				continue;

			buffer.back().push_back(c);
		}
	}

	void Fig::ParseValueIntoProperty(PropertyCollection::Iterator& propertyIt, const std::string& line)
	{
		//if (propertyIt == m_properties.end()) 
		if (!Assert(propertyIt != m_properties.EndMutable(), "Tried to parse value "
			"into property for line:{} but iterator points to END", line))
			return;

		ParseValue(propertyIt.GetValueMutable(), line);
		//propertyIt.GetValueMutable().push_back("");

		////Note: this is not too much worse than string.find since it is a naive search
		//bool foundSeparator = false;

		//for (const auto& c : line)
		//{
		//	if (c == VALUE_SEPARATOR_CHAR)
		//	{
		//		foundSeparator = true;
		//		propertyIt.GetValueMutable().push_back("");
		//		continue;
		//	}
		//	if (foundSeparator && c == ' ')
		//		continue;

		//	propertyIt.GetValueMutable().back().push_back(c);
		//}
	}

	//TODO: make property line parse value be a figvalue with vector rather than the full string
	Fig::PropertyParseResult Fig::ParsePropertyLine(const std::string& line, std::string* keyResult, std::string* valueResult)
	{
		const size_t keyValueSeparatorIndex = line.find(KEY_VALUE_SEPARATOR);
		if (keyValueSeparatorIndex == std::string::npos)
			return PropertyParseResult::NoKeyValueSeparator;

		//Note: we want to get rid of any spaces at the start or end of a key to make sure it can easily be searched/retrieved
		if (keyResult != nullptr) *keyResult = ::Utils::StringUtil(line.substr(0, keyValueSeparatorIndex)).TrimSpaces().ToString();

		//Note: we want to ignore any spaces between the initial key declaration and the start of the value
		const size_t valueStartIdx = line.find_first_not_of(' ', keyValueSeparatorIndex + 1);
		if (valueStartIdx == std::string::npos)
			return PropertyParseResult::NoPropertyValue;

		if (valueResult != nullptr) *valueResult = ::Utils::StringUtil(line.substr(valueStartIdx)).TrimSpaces().ToString();
		return PropertyParseResult::Success;
	}

	Fig::PropertyParseResult Fig::ParsePropertyLine(const std::string& line, std::string* keyResult, FigValue* valueResult)
	{
		std::string valueFull = "";
		Fig::PropertyParseResult parseResult = ParsePropertyLine(line, keyResult, &valueFull);
		if (parseResult != PropertyParseResult::Success || valueResult == nullptr) return parseResult;

		ParseValue(*valueResult, valueFull);
		return PropertyParseResult::Success;
	}

	bool Fig::HasComment(const std::string& line)
	{
		size_t startSymbolIdx = line.find(COMMENT_START);
		size_t endSymbolIdx = line.find(COMMENT_CLOSE);

		return startSymbolIdx != std::string::npos && endSymbolIdx != std::string::npos &&
			endSymbolIdx >= startSymbolIdx + COMMENT_START.size();
	}

	void Fig::ParseProperty(const std::string& line, const FigParseFlag flag)
	{
		std::string key = "";
		std::string value = "";
		//Note: we should not use the figvalue overload with vector of string values because
		//then we will have to make a copy of the vector when placing into target location
		//since we are not sure which iterator to add it to
		PropertyParseResult parseResult = ParsePropertyLine(line, &key, &value);

		//If we have no colon (meaning it is not a key value pair and is just a overflow value)
		//we just attempt to add to the most recent added property to allow
		//for multi-line data
		if (parseResult == PropertyParseResult::NoKeyValueSeparator)
		{
			if (!Assert(!m_properties.IsEmpty(), "Tried to add FIG property from line:'{}' with no KEY VALUE pair "
				"but that is only allowed if there are existing properties (there are 0)", line))
				return;

			//LogError(std::format("Line has no key: {}", line));
			//Note: if the line contains the spaces, we keep the line as it appeared, otherwise we trim
			const std::string lineFormatted = ::Utils::HasFlagAll(flag, FigParseFlag::IncludeOverflowLineStartSpaces) ?
				line : ::Utils::StringUtil(line).TrimSpaces().ToString();

			//Log(std::format("Line formatted is:{}", lineFormatted));
			ParseValueIntoProperty(--m_properties.EndMutable(), lineFormatted);
			return;
		}

		//Even if it is empty, we allow no values since it may carry over to the next line
		else if (parseResult == PropertyParseResult::NoPropertyValue)
		{
			m_properties.Insert(key, FigValue());
			return;
		}

		if (!Assert(parseResult == PropertyParseResult::Success, "Tried to parse FIG property from line:'{}' "
			"but parse resulted in a non-success state that was not handled", line))
			return;

		auto propertyIt = m_properties.Insert(key, FigValue());
		if (!Assert(propertyIt.second, "Tried to add FIG line:'{}' "
			"but properties failed to add key:{}", line, key))
			return;

		if (!Assert(propertyIt.first.GetKey() == key, "Tried to add FIG line:'{}' "
			"but the found key:{} does not match the iterator key:{} properties:{}", line, key, propertyIt.first.GetKey(), m_properties.ToString(true)))
			return;

		ParseValueIntoProperty(propertyIt.first, value);
	}
	void Fig::ParseHeaderProperty(const std::string& header, const std::string& line, const FigParseFlag flag)
	{
		MarkedPropertyCollection::iterator markedSectionIt = m_markedProperties.find(header);
		if (markedSectionIt == m_markedProperties.end())
		{
			std::pair<MarkedPropertyCollection::iterator, bool> createdProperty = m_markedProperties.emplace(header, new Fig());
			if (!Assert(createdProperty.second, "Tried to add a marked property section in "
				"FIG file with header:{} but failed to add", header))
				return;

			markedSectionIt = createdProperty.first;
		}

		//LogError(std::format("Adding marked proeprty:{} of line:{}", header, line));
		markedSectionIt->second->ParseProperty(line, flag);
	}

	void Fig::AddProperty(const std::string& propertyName, const std::string& propertyValue)
	{
		auto insertResult = m_properties.Insert(propertyName, FigValue());
		if (!Assert(insertResult.second, "Tried to add property: {} with value: {} but failed "
			"due to a property with that name already existing", propertyName, propertyValue))
			return;

		auto it = insertResult.first;
		ParseValueIntoProperty(it, propertyValue);
	}
	void Fig::AddHeaderProperty(const std::string& headerName, const std::string propertyName, const std::string& propertyValue)
	{
		auto it = m_markedProperties.find(headerName);
		if (it == m_markedProperties.end())
		{
			it = m_markedProperties.emplace(headerName, new Fig()).first;
		}
		it->second->AddProperty(propertyName, propertyValue);
	}

	void Fig::CreateFigFormat(const std::vector<std::string>& lineContents, const FigParseFlag flags)
	{
		std::string currentMarker = "";
		std::string cleanedLine = "";
		bool isInComment = false;

		for (auto& line : lineContents)
		{
			if (line.empty()) continue;

			cleanedLine = "";
			for (size_t i = 0; i < line.size(); i++)
			{
				if (i < line.size() - COMMENT_START.size() - COMMENT_CLOSE.size()
					&& line.substr(i, COMMENT_START.size()) == COMMENT_START)
					isInComment = true;

				else if (isInComment && i < line.size() - COMMENT_CLOSE.size()
					&& line.substr(i, COMMENT_CLOSE.size()) == COMMENT_CLOSE)
					isInComment = false;

				if (!isInComment) cleanedLine.push_back(line[i]);
			}
			if (cleanedLine.empty()) continue;
			////Note: we only want to trim tabs and NOT SPACES since spaces may be part of line
			//cleanedLine = ::Utils::StringUtil(cleanedLine).Trim().ToString();

			//Log(std::format("Found line:{}", cleanedLine));
			if (cleanedLine[0] == MARKER_CHAR)
			{
				currentMarker = cleanedLine.substr(1);
				if (!Assert(!currentMarker.empty(), "Tried to create FIG contents for line:'{}' "
					"but current marker is not complete", cleanedLine))
					return;

				if (!Assert(currentMarker.find(KEY_VALUE_SEPARATOR) == std::string::npos, "Tried to create FIG contents but line:{} "
					"contains marker:{} with invalid character '{}'", cleanedLine, currentMarker, ::Utils::ToString(KEY_VALUE_SEPARATOR)))
					return;

				continue;
			}

			if (currentMarker.empty()) ParseProperty(cleanedLine, flags);
			else ParseHeaderProperty(currentMarker, cleanedLine, flags);
		}
	}

	bool Fig::HasBaldProperty(const std::string& key) const
	{
		return m_properties.Find(key) != m_properties.End();
	}
	bool Fig::HasMarker(const std::string& markerName) const
	{
		return m_markedProperties.find(markerName) != m_markedProperties.end();
	}
	bool Fig::HasMarkedProperty(const std::string& markerName, const std::string& key) const
	{
		auto markerIt = m_markedProperties.find(markerName);
		if (markerIt == m_markedProperties.end()) return false;

		return markerIt->second->HasBaldProperty(key);
	}


	const FigValue& Fig::TryGetBaldValue(const std::string& key) const
	{
		auto propertyIt = m_properties.Find(key);
		if (propertyIt == m_properties.End()) return {};

		return propertyIt.GetValue();
	}
	const FigValue& Fig::TryGetMarkedValue(const std::string& markerName, const std::string& key) const
	{
		auto markerIt = m_markedProperties.find(markerName);
		if (markerIt == m_markedProperties.end()) return {};

		return markerIt->second->TryGetBaldValue(key);
	}

	void Fig::GetAllValues(std::vector<const FigValue*>& input) const
	{
		for (const auto& property : m_properties)
		{
			input.emplace_back(property.second);
		}

		if (m_markedProperties.empty())
			return;

		for (const auto& markedProperty : m_markedProperties)
		{
			if (markedProperty.second == nullptr) continue;
			markedProperty.second->GetAllValues(input);
		}
	}
	void Fig::GetAllValues(const std::string& markerName, std::vector<const FigValue*>& vector) const
	{
		auto markerIt = m_markedProperties.find(markerName);
		if (markerIt == m_markedProperties.end() || markerIt->second == nullptr)
			return;

		markerIt->second->GetAllValues(vector);
	}

	void Fig::GetAllValuesFrom(const std::string& markerName, std::vector<const FigValue*>& input) const
	{
		auto markerIt = m_markedProperties.find(markerName);
		if (markerIt == m_markedProperties.end() || markerIt->second == nullptr)
			return;

		for (auto& it = markerIt; it != m_markedProperties.end(); it++)
		{
			it->second->GetAllValues(input);
		}
	}

	void Fig::GetAllProperties(std::vector<FigPropertyRef>& properties) const
	{
		for (const auto& property : m_properties)
		{
			properties.emplace_back(*property.first, *property.second);
			/*LogError(std::format("Added property:{} val:{}", *property.first,
				Utils::ToStringIterable<FigValue, std::string>(*property.second)));*/
		}

		if (m_markedProperties.empty())
			return;

		for (const auto& markedProperty : m_markedProperties)
		{
			if (markedProperty.second == nullptr) continue;
			markedProperty.second->GetAllProperties(properties);
		}
	}
	void Fig::GetAllProperties(std::unordered_map<std::string, FigValue>& properties) const
	{
		properties = m_properties.AsUnorderedMap();

		if (m_markedProperties.empty())
			return;

		for (const auto& markedProperty : m_markedProperties)
		{
			if (markedProperty.second == nullptr) continue;
			markedProperty.second->GetAllProperties(properties);
		}
	}
	void Fig::GetAllProperties(const std::string& markerName, std::vector<FigPropertyRef>& properties) const
	{
		auto markerIt = m_markedProperties.find(markerName);
		if (markerIt == m_markedProperties.end() || markerIt->second == nullptr)
			return;

		markerIt->second->GetAllProperties(properties);
	}
	void Fig::GetAllProperties(const std::string& markerName, std::unordered_map<std::string, FigValue>& properties) const
	{
		auto markerIt = m_markedProperties.find(markerName);
		if (markerIt == m_markedProperties.end() || markerIt->second == nullptr)
			return;

		markerIt->second->GetAllProperties(properties);
	}

	std::optional<FigProperty> Fig::TryGetPropertyFromLine(const std::string& line)
	{
		FigProperty resultProperty = FigProperty();
		ParsePropertyLine(line, &resultProperty.m_Key, &resultProperty.m_Value);
		return resultProperty.IsEmpty() ? std::nullopt : std::make_optional<FigProperty>(resultProperty);
	}

	std::string Fig::ToString() const
	{
		std::string result = "";
		for (const auto& property : m_properties)
		{
			result += std::format("[{}:{}]", property.first == nullptr ? "NULL" : *property.first,
				property.second == nullptr ? "NULL" : ::Utils::ToStringIterable(*property.second));
		}

		if (m_markedProperties.empty())
			return result;

		for (const auto& markedProperty : m_markedProperties)
		{
			if (markedProperty.second == nullptr) continue;

			result += std::format(" @{}", markedProperty.first);
			result += markedProperty.second->ToString();
		}
		return result;
	}
	void Fig::ToFormattedString(std::string& outVal) const
	{
		for (const auto& property : m_properties)
		{
			outVal += std::format("{}{}", *property.first, KEY_VALUE_SEPARATOR);
			const FigValue& propertyVals = *property.second;
			if (propertyVals.empty()) continue;

			outVal += propertyVals[0];
			size_t propertyValCount = property.second->size();
			for (size_t i=1; i< propertyValCount; i++)
			{
				outVal += std::format("{} {}", VALUE_SEPARATOR_CHAR, propertyVals[i]);
			}
			outVal += "\n";
		}

		if (m_markedProperties.empty())
			return;

		for (const auto& markedProperty : m_markedProperties)
		{
			outVal += std::format("{}{}\n", MARKER_CHAR, markedProperty.first); 
			markedProperty.second->ToFormattedString(outVal);
		}
	}

	void Fig::WriteToPath(const std::filesystem::path& path) const
	{
		std::string formattedStr = "";
		ToFormattedString(formattedStr);
		Utils::IO::TryWriteFile(path, formattedStr);
	}

	void FromFigValue(const std::string& value, bool& outVal)
	{
		outVal = (::Utils::StringUtil(value).ToLowerCase().ToString() == BOOL_VALUE_TRUE);
	}
	void ToFigValue(const bool value, std::string& outVal)
	{
		outVal += (value) ? BOOL_VALUE_TRUE : BOOL_VALUE_FALSE;
	}
	bool ToBool(const std::string& value)
	{
		bool outVal = false;
		FromFigValue(value, outVal);
		return outVal;
	}

	void FromFigValue(const std::string& value, int& outVal)
	{
		outVal = ::Utils::TryParse<int>(value).value_or(0);
	}
	void ToFigValue(const int value, std::string& outVal)
	{
		outVal += std::to_string(value);
	}
	int ToInt32(const std::string& value)
	{
		int outVal = 0;
		FromFigValue(value, outVal);
		return outVal;
	}

	void FromFigValue(const std::string& value, float& outVal)
	{
		outVal = ::Utils::TryParse<float>(value).value_or(0);
	}
	void ToFigValue(const float value, std::string& outVal)
	{
		outVal += ::Utils::ToStringRoundTrip(value);
	}
	float ToFloat(const std::string& value)
	{
		float outVal = 0;
		FromFigValue(value, outVal);
		return outVal;
	}
	
	void FromFigValue(const std::string& value, std::uint8_t& outVal)
	{
		outVal = ::Utils::TryParse<std::uint8_t>(value).value_or(0);
	}
	void ToFigValue(const std::uint8_t value, std::string& outVal)
	{
		outVal += std::to_string(value);
	}
	std::uint8_t ToUint8(const std::string& value)
	{
		std::uint8_t outVal = 0;
		FromFigValue(value, outVal);
		return outVal;
	}

	bool IsNull(const std::string& value, bool trimSpaces)
	{
		if (trimSpaces)
		{
			Utils::StringUtil stringFormatter = Utils::StringUtil(value);
			std::string cleanedValue = stringFormatter.TrimAnySpaceChar().ToString();
			return cleanedValue == Fig::NULL_VALUE;
		}
		else return value == Fig::NULL_VALUE;
	}
}
