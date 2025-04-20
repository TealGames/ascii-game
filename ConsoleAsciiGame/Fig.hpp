#pragma once
#include <string>
#include <filesystem>
#include "PreservedMap.hpp"

using FigValue = std::vector<std::string>;

struct FigProperty
{
	const char* m_Key;
	const FigValue* m_Value;

	FigProperty(const std::string& key, const FigValue& value);
	//Note: we do this to prevent rvalue assignments so we can optimize
	//with no copying with lvalue references
	FigProperty(std::string&&, FigValue&&) = delete;

	std::string GetKey() const;
	const FigValue& GetValue() const;

	std::string ToString() const;
};

/// <summary>
/// FIG is a data serialization format made for configuration files and simple asset serialization
/// especially with not many nested levels, with a purpose of being lightweight and easily extensible
/// </summary>
using PropertyCollection = PreservedMap<std::string, FigValue>;
class Fig;
using MarkedPropertyCollection = std::unordered_map<std::string, Fig*>;

class Fig
{
private:
	PropertyCollection m_properties;
	MarkedPropertyCollection m_markedProperties;

public:
	static constexpr char MARKER_CHAR = '@';
	static constexpr char VALUE_SEPARATOR_CHAR = ',';
	static constexpr char KEY_VALUE_SEPARATOR = ':';

private:
	void ParseValueIntoProperty(PropertyCollection::Iterator& propertyIt, const std::string& line);
	void AddProperty(const std::string& line);
	void AddMarkedProperty(const std::string& header, const std::string& line);

public:
	Fig();
	Fig(const std::string& contents);
	Fig(const std::filesystem::path& path);
	~Fig();

	void CreateContents(const std::vector<std::string>& str);

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

	void GetAllProperties(std::vector<FigProperty>& properties) const;
	void GetAllProperties(const std::string& markerName, std::vector<FigProperty>& properties) const;

	std::string ToString() const;
};

