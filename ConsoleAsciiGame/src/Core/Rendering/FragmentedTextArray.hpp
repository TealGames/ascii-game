#pragma once
#include "TextArray.hpp"

enum class TextArrayPositionType
{
	ArrayIndex,
	PositionData,
};

std::string ToString(const TextArrayPositionType& positionType);

using TextCharCollection = std::vector<std::vector<TextCharArrayPosition>>;
class FragmentedTextArray
{
private:
	TextCharCollection m_textArray;
	//These are used for having some optimization to position lookup
	Array2DPosition m_minIndices;
	Array2DPosition m_maxIndices;
public:

private:
	TextCharArrayPosition* TryGetPosMutable(const Array2DPosition& position, const TextArrayPositionType& positionType);

public:
	FragmentedTextArray();
	FragmentedTextArray(const std::vector<std::vector<TextCharArrayPosition>>& chars);

	
	/// <summary>
	/// Returns true if the position exists within the array data
	/// </summary>
	/// <param name="rowColPos"></param>
	/// <returns></returns>
	bool DoesPositionExist(const Array2DPosition& rowColPos);
	/// <summary>
	/// Returns true if the position is a valid [ROW, COL] position in the array
	/// </summary>
	/// <param name="rowColPos"></param>
	/// <returns></returns>
	bool IsValidIndexPos(const Array2DPosition& index);

	const TextCharArrayPosition* TryGetPos(const Array2DPosition& rowColPos, const TextArrayPositionType& positionType);

	void SetAt(const Array2DPosition& pos, const TextArrayPositionType& positionTypen, const TextChar& newBufferChar);
	void SetAt(const Array2DPosition& pos, const TextArrayPositionType& positionTypen, const char& newChar);
	void SetAt(const Array2DPosition& pos, const TextArrayPositionType& positionTypen, const Color& newColor);

	void SetAt(const std::vector<Array2DPosition>& rowColPos, const TextArrayPositionType& positionTypen, const TextChar& newBufferChar);
	void SetAt(const std::vector<TextCharArrayPosition>& updatedCharsAtPos, const TextArrayPositionType& positionTypen);
	void SetAt(const std::vector<ColorPosition>& updateColorsAtPos, const TextArrayPositionType& positionTypen);

	static std::string ToString(const std::vector<std::vector<TextCharArrayPosition>>& chars,
		const bool convertChars = true);
	std::string ToString(const bool convertChars = true) const;

	FragmentedTextArray& operator=(const FragmentedTextArray& other);
	FragmentedTextArray& operator=(FragmentedTextArray&& other) noexcept;
};

