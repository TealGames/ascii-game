#pragma once
#include <string>
#include <filesystem>
#include "Utils/DataStructure/PreservedMap.hpp"
#include <cstdint>

using FigValue = std::vector<std::string>;

/// <summary>
/// Stores a reference to an existing fig key and value
/// </summary>
struct FigPropertyRef
{
	const char* m_Key;
	const FigValue* m_Value;

	FigPropertyRef(const std::string& key, const FigValue& value);
	//Note: we do this to prevent rvalue assignments so we can optimize
	//with no copying with lvalue references
	FigPropertyRef(std::string&&, FigValue&&) = delete;

	std::string GetKey() const;
	const FigValue& GetValue() const;

	std::string ToString() const;
};

struct FigProperty
{
	std::string m_Key;
	FigValue m_Value;

	FigProperty();
	FigProperty(const std::string& key, const FigValue& value);

	bool IsEmpty() const;

	std::string ToString() const;
};

/// <summary>
/// FIG is a data serialization format made for configuration files and simple asset serialization
/// especially with not many nested levels, with a purpose of being lightweight and easily extensible
/// </summary>
using PropertyCollection = PreservedMap<std::string, FigValue>;
class Fig;
using MarkedPropertyCollection = std::unordered_map<std::string, Fig*>;

using FigFlagIntegralType = std::uint8_t;
enum class FigFlag : FigFlagIntegralType
{
	None= 0,
	/// <summary>
	/// Will include spaces at the start of each new line of a property value's (ONLY new lines after initial property key declaration line
	/// so spaces between key and value of initial line will NOT be counted, only the lines that overflow)
	/// as part of the value itself up until the first character of that line. By DEFAULT, overflow line's spaces at the start are trimmed
	/// </summary>
	IncludeOverflowLineStartSpaces= 1<<0
};

constexpr FigFlag operator&(const FigFlag& lhs, const FigFlag& rhs)
{
	return static_cast<FigFlag>(static_cast<FigFlagIntegralType>(lhs)
		& static_cast<FigFlagIntegralType>(rhs));
}
constexpr FigFlag& operator&=(FigFlag& lhs, const FigFlag& rhs)
{
	lhs = lhs & rhs;
	return lhs;
}
constexpr FigFlag operator|(const FigFlag& lhs, const FigFlag& rhs)
{
	return static_cast<FigFlag>(static_cast<FigFlagIntegralType>(lhs)
		| static_cast<FigFlagIntegralType>(rhs));
}
constexpr FigFlag& operator|=(FigFlag& lhs, const FigFlag& rhs)
{
	lhs = lhs | rhs;
	return lhs;
}
constexpr FigFlag operator~(const FigFlag& op)
{
	return static_cast<FigFlag>(~static_cast<FigFlagIntegralType>(op));
}

class Fig
{
private:
	enum class PropertyParseResult
	{
		Success,
		NoKeyValueSeparator,
		NoPropertyValue,
	};

	PropertyCollection m_properties;
	MarkedPropertyCollection m_markedProperties;

public:
	static constexpr char MARKER_CHAR = '@';
	static constexpr char VALUE_SEPARATOR_CHAR = ',';
	static constexpr char KEY_VALUE_SEPARATOR = ':';
	static const std::string COMMENT_START;
	static const std::string COMMENT_CLOSE;

private:
	static void ParseValue(std::vector<std::string>& buffer, const std::string& line);
	void ParseValueIntoProperty(PropertyCollection::Iterator& propertyIt, const std::string& line);
	static PropertyParseResult ParsePropertyLine(const std::string& line, std::string* keyResult, std::string* valueResult);
	static PropertyParseResult ParsePropertyLine(const std::string& line, std::string* keyResult, FigValue* valueResult);

	static bool HasComment(const std::string& line);

	void AddProperty(const std::string& line, const FigFlag flag = FigFlag::None);
	void AddMarkedProperty(const std::string& header, const std::string& line, const FigFlag flag = FigFlag::None);

public:
	Fig();
	Fig(const std::string& contents);
	Fig(const std::filesystem::path& path, const FigFlag flag= FigFlag::None);
	~Fig();

	void CreateContents(const std::vector<std::string>& str, const FigFlag flags= FigFlag::None);

	bool HasBaldProperty(const std::string& key) const;
	bool HasMarkedProperty(const std::string& markerName, const std::string& key) const;
	bool HasMarker(const std::string& markerName) const;

	const FigValue& TryGetBaldValue(const std::string& key) const;
	const FigValue& TryGetMarkedValue(const std::string& markerName, const std::string& key) const;

	void GetAllValues(std::vector<const FigValue*>& input) const;
	void GetAllValues(const std::string& markerName, std::vector<const FigValue*>& input) const;
	/// <summary>
	/// Will get all the properties starting from this marker name until the end RECURSIVELY (meaning includes tested)
	/// </summary>
	/// <param name="markerName"></param>
	/// <param name="input"></param>
	void GetAllValuesFrom(const std::string& markerName, std::vector<const FigValue*>& input) const;

	void GetAllProperties(std::vector<FigPropertyRef>& properties) const;
	void GetAllProperties(const std::string& markerName, std::vector<FigPropertyRef>& properties) const;

	static std::optional<FigProperty> TryGetPropertyFromLine(const std::string& line);

	std::string ToString() const;
};

