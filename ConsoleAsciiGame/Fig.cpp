#include "pch.hpp"
#include "Fig.hpp"
#include "HelperFunctions.hpp"
#include <fstream>
#include "StringUtil.hpp"
#include "Debug.hpp"

FigProperty::FigProperty(const std::string& key, const FigValue& value) 
	: m_Key(key.c_str()), m_Value(&value) {}

std::string FigProperty::GetKey() const
{
	return m_Key;
}
const FigValue& FigProperty::GetValue() const
{
	if (!Assert(this, m_Value != nullptr, std::format("Tried to get value from FIG property with key:{} "
		"but value is null", GetKey())))
		throw std::invalid_argument("Invalid value state");

	return *m_Value;
}

std::string FigProperty::ToString() const
{
	return std::format("[Key:{} Value:{}]", GetKey(), Utils::ToStringIterable<FigValue, std::string>(GetValue()));
}

Fig::Fig() : m_properties(), m_markedProperties() {}
Fig::Fig(const std::string& contents) : Fig()
{
	std::vector<std::string> fileLines = {""};
	size_t lineIndex = 0;
	for (const char& c : contents)
	{
		if (c == '\n')
		{
			fileLines.push_back("");
			lineIndex++;
			continue;
		}
		fileLines[lineIndex].push_back(c);
	}
	CreateContents(fileLines);
}
Fig::Fig(const std::filesystem::path& path) : Fig()
{
	std::string currentLine = "";
	std::fstream fstream = std::fstream(path);
	std::vector<std::string> fileLines = {};

	while (std::getline(fstream, currentLine))
	{
		fileLines.push_back(currentLine);
	}
	CreateContents(fileLines);
}
Fig::~Fig()
{
	for (auto& markedSection : m_markedProperties)
	{
		delete markedSection.second;
	}
	m_markedProperties = {};
}

void Fig::ParseValueIntoProperty(PropertyCollection::Iterator& propertyIt, const std::string& line)
{
	//if (propertyIt == m_properties.end()) 
	if (!Assert(this, propertyIt != m_properties.EndMutable(), std::format("Tried to parse value "
		"into property for line:{} but iterator points to END", line)))
		return;

	//Be default we always want to push a new value since if it is empty -> we want new val
	//if we need a new line -> we want to push new line
	//propertyIt->second.push_back("");
	propertyIt.GetValueMutable().push_back("");

	//Note: this is not too much worse than string.find since it is a naive search
	bool foundSeparator = false;

	for (const auto& c : line)
	{
		if (c == VALUE_SEPARATOR_CHAR)
		{
			foundSeparator = true;
			propertyIt.GetValueMutable().push_back("");
			continue;
		}
		if (foundSeparator && c == ' ')
			continue;

		propertyIt.GetValueMutable().back().push_back(c);
	}

	//LogError(std::format("Created new property: {} line:{} FULL FIG:{}", propertyIt.ToString(), line, ToString()));
}

void Fig::AddProperty(const std::string& line)
{
	const size_t colonIndex = line.find(KEY_VALUE_SEPARATOR);

	//If we have no colon (meaning it is not a key value pair)
	//we just attempt to add to the most recent added property (to allow
	//for multi-line data)
	if (colonIndex == std::string::npos)
	{
		if (!Assert(this, !m_properties.IsEmpty(), std::format("Tried to add FIG property from line:'{}' with no KEY VALUE pair "
			"but that is only allowed if there are existing properties (there are 0)", line)))
			return;

		//LogError(std::format("Line has no key: {}", line));
		ParseValueIntoProperty(--m_properties.EndMutable(), line);
		return;
	}

	const std::string& key = line.substr(0, colonIndex);
	const size_t valueStartIdx = line.find_first_not_of(' ', colonIndex+1);
	//Even if it is empty, we allow no values since it may carry over to the next line
	if (valueStartIdx == std::string::npos)
	{
		m_properties.Insert(key, FigValue());
		return;
	}

	const std::string valueFull = Utils::StringUtil(line.substr(valueStartIdx)).Trim().ToString();
	
	//We create a property with no value so that the parsed value can be added via function
	LogError(std::format("BEFORE INSERT properties:{}", m_properties.ToString(true)));
	auto propertyIt= m_properties.Insert(key, FigValue());
	//LogError(std::format("Added temp key:{} waiting for parse value property key:{}", key, propertyIt.first.GetKey()));
	if (!Assert(this, propertyIt.second, std::format("Tried to add FIG line:'{}' "
		"but properties failed to add key:{}", line, key)))
		return;

	if (!Assert(this, propertyIt.first.GetKey() == key, std::format("Tried to add FIG line:'{}' "
		"but the found key:{} does not match the iterator key:{} properties:{}", line, key, propertyIt.first.GetKey(), m_properties.ToString(true))))
		return;

	ParseValueIntoProperty(propertyIt.first, valueFull);
}
void Fig::AddMarkedProperty(const std::string& header, const std::string& line)
{
	MarkedPropertyCollection::iterator markedSectionIt = m_markedProperties.find(header);
	if (markedSectionIt == m_markedProperties.end())
	{
		std::pair<MarkedPropertyCollection::iterator, bool> createdProperty = m_markedProperties.emplace(header, new Fig());
		if (!Assert(this, createdProperty.second, std::format("Tried to add a marked property section in "
			"FIG file with header:{} but failed to add", header)))
			return;

		markedSectionIt = createdProperty.first;
	}
	
	//LogError(std::format("Adding marked proeprty:{} of line:{}", header, line));
	markedSectionIt->second->AddProperty(line);
}

void Fig::CreateContents(const std::vector<std::string>& contents)
{
	std::string currentMarker = "";
	for (auto line : contents)
	{
		if (line.empty()) continue;

		line = Utils::StringUtil(line).Trim().ToString();
		Log(std::format("Found line:{}", line));
		if (line[0] == MARKER_CHAR)
		{
			currentMarker = line.substr(1);
			if (!Assert(this, !currentMarker.empty(), std::format("Tried to create FIG contents for line:'{}' "
				"but current marker is not complete", line)))
				return;

			if (!Assert(this, !currentMarker.contains(KEY_VALUE_SEPARATOR), std::format("Tried to create FIG contents but line:{} "
				"contains marker:{} with invalid character '{}'", line, currentMarker, Utils::ToString(KEY_VALUE_SEPARATOR))))
				return;

			continue;
		}

		if (currentMarker.empty()) AddProperty(line);
		else AddMarkedProperty(currentMarker, line);
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
	if (markerIt == m_markedProperties.end() ) return {};

	return markerIt->second->TryGetBaldValue(key);
}

void Fig::GetAllValues(std::vector<const FigValue*>& input) const
{
	for (const auto& property : m_properties)
	{
		input.push_back(property.second);
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
	if (markerIt == m_markedProperties.end() || markerIt->second==nullptr) 
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

void Fig::GetAllProperties(std::vector<FigProperty>& properties) const
{
	for (const auto& property : m_properties)
	{
		properties.emplace_back(*property.first, *property.second);
		LogError(std::format("Added property:{} val:{}", *property.first,
			Utils::ToStringIterable<FigValue, std::string>(*property.second)));
	}

	if (m_markedProperties.empty())
		return;

	for (const auto& markedProperty : m_markedProperties)
	{
		if (markedProperty.second == nullptr) continue;
		markedProperty.second->GetAllProperties(properties);
	}
}
void Fig::GetAllProperties(const std::string& markerName, std::vector<FigProperty>& properties) const
{
	auto markerIt = m_markedProperties.find(markerName);
	if (markerIt == m_markedProperties.end() || markerIt->second == nullptr)
		return;

	markerIt->second->GetAllProperties(properties);
}

std::string Fig::ToString() const
{
	std::string result = "";
	for (const auto& property : m_properties)
	{
		result += std::format("[{}:{}]", property.first==nullptr? "NULL":  *property.first,
			property.second == nullptr ? "NULL" : Utils::ToStringIterable<FigValue, std::string>(*property.second));
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