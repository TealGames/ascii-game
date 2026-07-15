#pragma once
#include <string>
#include <filesystem>
#include "Utils/DataStructure/PreservedMap.hpp"
#include <cstdint>

namespace FigFormat
{
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

	using PropertyCollection = ::Utils::PreservedMap<std::string, FigValue>;
	class Fig;
	using MarkedPropertyCollection = std::unordered_map<std::string, Fig*>;

	template<typename T>
	concept IsFigConvertible = requires(const std::string& constFig, T& outMutableT,
										const T& constT, std::string& outMutableFig) 
	{ 
		{ FromFigValue(constFig, outMutableT) } -> std::same_as<void>;
		{ ToFigValue(constT, outMutableFig) }-> std::same_as<void>;
	};

	using FigFlagIntegralType = std::uint8_t;
	enum class FigParseFlag : FigFlagIntegralType
	{
		None = 0,
		/// <summary>
		/// Will include spaces at the start of each new line of a property value's (ONLY new lines after initial property key declaration line
		/// so spaces between key and value of initial line will NOT be counted, only the lines that overflow)
		/// as part of the value itself up until the first character of that line. By DEFAULT, overflow line's spaces at the start are trimmed
		/// </summary>
		IncludeOverflowLineStartSpaces = 1 << 0
	};

	constexpr FigParseFlag operator&(const FigParseFlag& lhs, const FigParseFlag& rhs)
	{
		return static_cast<FigParseFlag>(static_cast<FigFlagIntegralType>(lhs)
			& static_cast<FigFlagIntegralType>(rhs));
	}
	constexpr FigParseFlag& operator&=(FigParseFlag& lhs, const FigParseFlag& rhs)
	{
		lhs = lhs & rhs;
		return lhs;
	}
	constexpr FigParseFlag operator|(const FigParseFlag& lhs, const FigParseFlag& rhs)
	{
		return static_cast<FigParseFlag>(static_cast<FigFlagIntegralType>(lhs)
			| static_cast<FigFlagIntegralType>(rhs));
	}
	constexpr FigParseFlag& operator|=(FigParseFlag& lhs, const FigParseFlag& rhs)
	{
		lhs = lhs | rhs;
		return lhs;
	}
	constexpr FigParseFlag operator~(const FigParseFlag& op)
	{
		return static_cast<FigParseFlag>(~static_cast<FigFlagIntegralType>(op));
	}

	template<typename T>
	struct FigSerializer;

	/// <summary>
	/// FIG is a data serialization format made for configuration files and simple asset serialization
	/// especially with not many nested levels, with a purpose of being lightweight and easily extensible
	/// </summary>
	class Fig
	{
	private:
		enum class PropertyParseResult : std::uint8_t
		{
			Success					= 0,
			NoKeyValueSeparator		= 1,
			NoPropertyValue			= 2,
		};

		PropertyCollection m_properties;
		MarkedPropertyCollection m_markedProperties;

	public:
		static constexpr char MARKER_CHAR = '@';
		static constexpr char VALUE_SEPARATOR_CHAR = ',';
		static constexpr char KEY_VALUE_SEPARATOR = ':';
		static constexpr const char* NULL_VALUE = "null";
		static const std::string COMMENT_START;
		static const std::string COMMENT_CLOSE;

	private:
		static void ParseValue(std::vector<std::string>& buffer, const std::string& line);
		void ParseValueIntoProperty(PropertyCollection::Iterator& propertyIt, const std::string& line);
		static PropertyParseResult ParsePropertyLine(const std::string& line, std::string* keyResult, std::string* valueResult);
		static PropertyParseResult ParsePropertyLine(const std::string& line, std::string* keyResult, FigValue* valueResult);

		static bool HasComment(const std::string& line);
		
		/// <summary>
		/// Will parse a string property as a line into the fig internal storage
		/// </summary>
		/// <param name="line"></param>
		/// <param name="flag"></param>
		void ParseProperty(const std::string& line, const FigParseFlag flag = FigParseFlag::None);

		/// <summary>
		/// Will parse a string property that belongs to a labeled region 
		/// with a header into the fig internal storage
		/// </summary>
		/// <param name="header"></param>
		/// <param name="line"></param>
		/// <param name="flag"></param>
		void ParseHeaderProperty(const std::string& header, const std::string& line, const FigParseFlag flag = FigParseFlag::None);

	public:
		Fig();
		Fig(const std::string& contents);
		Fig(const std::filesystem::path& path, const FigParseFlag flag = FigParseFlag::None);
		~Fig();

		void CreateFigFormat(const std::vector<std::string>& lineContents, const FigParseFlag flags = FigParseFlag::None);

		void AddProperty(const std::string& propertyName, const std::string& propertyValue);
		void AddHeaderProperty(const std::string& headerName, const std::string propertyName, const std::string& propertyValue);

		template<typename T>
		//requires (IsFigConvertible<T>)
		void AddProperty(const std::string& propertyName, const T& propertyValue)
		{
			std::string outFigVal = "";
			FigSerializer<T>::ToFigValue(propertyValue, outFigVal);
			AddProperty(propertyName, outFigVal);
		}
		template<typename T>
		//requires (IsFigConvertible<T>)
		void AddHeaderProperty(const std::string& headerName, const std::string propertyName, const T& propertyValue)
		{
			std::string outFigVal = "";
			FigSerializer<T>::ToFigValue(propertyValue, outFigVal);
			AddHeaderProperty(headerName, propertyName, outFigVal);
		}

		bool HasBaldProperty(const std::string& propertyName) const;
		bool HasMarkedProperty(const std::string& markerName, const std::string& propertyName) const;
		bool HasMarker(const std::string& markerName) const;

		const FigValue& TryGetBaldValue(const std::string& propertyName) const;
		const FigValue& TryGetMarkedValue(const std::string& markerName, const std::string& propertyName) const;

		template<typename T>
		void TryGetBaldValue(const std::string& propertyName, T& outVal) const
		{
			const FigValue& figVal = TryGetBaldValue(propertyName);
			if (figVal.empty())
				return;

			FigSerializer<T>::FromFigValue(figVal[0], outVal);
		}
		template<typename T>
		void TryGetMarkedValue(const std::string& markerName, const std::string& propertyName, T& outVal) const
		{
			const FigValue& figVal = TryGetMarkedValue(markerName, propertyName);
			if (figVal.empty())
				return;

			FigSerializer<T>::FromFigValue(figVal[0], outVal);
		}

		void GetAllValues(std::vector<const FigValue*>& input) const;
		void GetAllValues(const std::string& markerName, std::vector<const FigValue*>& input) const;
		/// <summary>
		/// Will get all the properties starting from this marker name until the end RECURSIVELY (meaning includes tested)
		/// </summary>
		/// <param name="markerName"></param>
		/// <param name="input"></param>
		void GetAllValuesFrom(const std::string& markerName, std::vector<const FigValue*>& input) const;

		void GetAllProperties(std::vector<FigPropertyRef>& properties) const;
		void GetAllProperties(std::unordered_map<std::string, FigValue>& properties) const;
		void GetAllProperties(const std::string& markerName, std::vector<FigPropertyRef>& properties) const;
		void GetAllProperties(const std::string& markerName, std::unordered_map<std::string, FigValue>& properties) const;

		static std::optional<FigProperty> TryGetPropertyFromLine(const std::string& line);

		std::string ToString() const;
		void ToFormattedString(std::string& outVal) const;
		void WriteToPath(const std::filesystem::path& path) const;
	};

	void FromFigValue(const std::string& value, bool& outVal);
	void ToFigValue(const bool value, std::string& outVal);
	bool ToBool(const std::string& value);

	void FromFigValue(const std::string& value, int& outVal);
	void ToFigValue(const int value, std::string& outVal);
	int ToInt32(const std::string& value);

	void FromFigValue(const std::string& value, float& outVal);
	void ToFigValue(const float value, std::string& outVal);
	float ToFloat(const std::string& value);

	void FromFigValue(const std::string& value, std::uint8_t& outVal);
	void ToFigValue(const std::uint8_t value, std::string& outVal);
	std::uint8_t ToUint8(const std::string& value);

#define BUILTIN_FIG_SERIALIZER(Type) \
	template<> \
	struct FigSerializer<Type> \
	{ \
		static void FromFigValue(const std::string& value, Type& outVal) { FigFormat::FromFigValue(value, outVal); } \
		static void ToFigValue(const Type value, std::string& outVal) { FigFormat::ToFigValue(value, outVal); } \
	};

	BUILTIN_FIG_SERIALIZER(bool)
	BUILTIN_FIG_SERIALIZER(int)
	BUILTIN_FIG_SERIALIZER(float)
	BUILTIN_FIG_SERIALIZER(std::uint8_t)

	template<typename T, size_t N>
		requires (N >= 1)
	struct FigSerializer<std::array<T, N>>
	{
		static void FromFigValue(const std::string& value, std::array<T, N>& outVal)
		{
			std::uint8_t arrIndex = 0;
			std::string componentStr = "";
			for (std::uint32_t i = 0; i < value.length(); i++)
			{
				const bool isSpace = (value[i] == ' ');
				if (isSpace) continue;
				//If the previous element was a space and we are not at a space, increase index
				if (i > 0 && value[i - 1] == ' ' && !isSpace)
				{
					FigSerializer<T>::FromFigValue(componentStr, outVal[arrIndex]);
					arrIndex++;
					componentStr = "";
				}
				componentStr += value[i];
			}
		}
		static void ToFigValue(const std::array<T, N>& value, std::string& outVal)
		{
			FigSerializer<T>::ToFigValue(value[0], outVal);
			for (size_t i = 1; i < N; i++)
			{
				outVal += " ";
				FigSerializer<T>::ToFigValue(value[i], outVal);
			}
		}
	};
	
	template<typename T, size_t N>
		requires (N >= 1)
	std::array<T, N> ToVec(const std::string& value)
	{
		std::array<T, N> arr = {};
		FigSerializer<std::array<T, N>>::FromFigValue(value, arr);
		return arr;
	}

	bool IsNull(const std::string& value, bool trimSpaces = true);
}


