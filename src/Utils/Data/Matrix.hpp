#pragma once
#include <array>
#include <cstdint>
#include <format>
#include "Vec4Type.hpp"

//TODO: add other matrix operations like inversion, row swapping, gausian elimination, determinant

template<size_t ROW_SIZE, size_t COL_SIZE>
class MatrixType
{
private:
	/// <summary>
	/// Note: elements are stored in COLUMN MAJOR ORDER:
	/// this means [0][0]..[0][3] is FIRST COLUMN, [1][0]..[1][3] is SECOND COLUMN
	/// </summary>
	Vec<float, COL_SIZE> m_elements[ROW_SIZE];
public:
	static constexpr size_t MATRIX_SIZE = ROW_SIZE * COL_SIZE;

private:
public:
	constexpr MatrixType() : MatrixType({}) {}
	constexpr MatrixType(const float rowMajorElements[ROW_SIZE][COL_SIZE]) : m_elements() 
	{
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			for (size_t c = 0; c < COL_SIZE; c++)
			{
				m_elements[c][r] = rowMajorElements[r][c];
			}
		}
	}
	constexpr MatrixType(const std::array<std::array<float, COL_SIZE>, ROW_SIZE>& rowMajorElements) : m_elements()
	{
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			for (size_t c = 0; c < COL_SIZE; c++)
			{
				m_elements[c][r] = rowMajorElements[r][c];
			}
		}
	}

	const float* GetMemPointer() const
	{
		return m_elements;
	}

	std::array<float, MATRIX_SIZE> GetElementsRowMajor() const
	{
		std::array<float, MATRIX_SIZE> arr = {};
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			for (size_t c = 0; c < COL_SIZE; c++)
			{
				arr[r * ROW_SIZE + c] = m_elements[c][r];
			}
		}
		return arr;
	}
	std::array<float, MATRIX_SIZE> GetElementsColMajor() const
	{
		std::array<float, MATRIX_SIZE> arr = {};
		for (size_t c = 0; c < COL_SIZE; c++)
		{
			for (size_t r = 0; r < ROW_SIZE; r++)
			{
				arr[c * COL_SIZE + r] = m_elements[c][r];
			}
		}
		return m_elements;
	}

	std::array<std::array<float, COL_SIZE>, ROW_SIZE> GetElements2DRowMajor() const
	{
		std::array<std::array<float, COL_SIZE>, ROW_SIZE> arr = {};
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			for (size_t c = 0; c < COL_SIZE; c++)
			{
				arr[r][c] = m_elements[c][r];
			}
		}
		return arr;
	}
	std::array<std::array<float, ROW_SIZE>, COL_SIZE> GetElements2DColMajor() const
	{
		return m_elements;
	}

	float Get(const std::uint8_t r, const std::uint8_t c) const
	{
		if (r >= ROW_SIZE || c >= COL_SIZE)
		{
			throw std::invalid_argument(std::format("Invalid matrix[{}][{}] get access:({}, {})", ROW_SIZE, COL_SIZE, r, c));
			return 0;
		}
		return m_elements[c][r];
	}
	std::array<float, COL_SIZE> GetRowArray(const std::uint8_t r) const
	{
		if (r >= ROW_SIZE)
		{
			throw std::invalid_argument(std::format("Invalid matrix[{}][{}] row get access:{}", ROW_SIZE, COL_SIZE, r));
			return {};
		}
		std::array<float, COL_SIZE> result = {};
		for (size_t c = 0; c < COL_SIZE; c++)
		{
			result[c] = m_elements[c][r];
		}
		return result;
	}
	Vec<float, COL_SIZE> GetRowVector(const std::uint8_t r) const
	{
		if (r >= ROW_SIZE)
		{
			throw std::invalid_argument(std::format("Invalid matrix[{}][{}] row get access:{}", ROW_SIZE, COL_SIZE, r));
			return {};
		}
		Vec<float, COL_SIZE> result = {};
		for (size_t c = 0; c < COL_SIZE; c++)
		{
			result[c] = m_elements[c][r];
		}
		return result;
	}
	std::array<float, ROW_SIZE> GetColArray(const std::uint8_t c) const
	{
		if (c >= COL_SIZE)
		{
			throw std::invalid_argument(std::format("Invalid matrix[{}][{}] col get access:{}", ROW_SIZE, COL_SIZE, c));
			return {};
		}
		std::array<float, ROW_SIZE> result = {};
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			result[r] = m_elements[c][r];
		}
		return result;
	}
	Vec<float, ROW_SIZE> GetColVector(const std::uint8_t c) const
	{
		if (c >= COL_SIZE)
		{
			throw std::invalid_argument(std::format("Invalid matrix[{}][{}] row get access:{}", ROW_SIZE, COL_SIZE, c));
			return {};
		}
		return m_elements[c];
	}


	void Set(const std::uint8_t r, const std::uint8_t c, const float newVal)
	{
		if (r >= ROW_SIZE || c >= COL_SIZE)
		{
			throw std::invalid_argument(std::format("Invalid matrix[{}][{}] set access:({}, {})", ROW_SIZE, r, c));
			return;
		}
		m_elements[c][r] = newVal;
	}

	/// <summary>
	/// Transpose moves the elements so an element at pos [R, C] -> [C, R]
	/// </summary>
	/// <returns></returns>
	MatrixType<COL_SIZE, ROW_SIZE> Transpose() const
	{
		MatrixType<COL_SIZE, ROW_SIZE> result = {};
		for (size_t c = 0; c < COL_SIZE; c++)
		{
			for (size_t r = 0; r < ROW_SIZE; r++)
			{
				result.m_elements[r][c] = m_elements[c][r];
			}
		}
		return result;
	}

	MatrixType GetIdentity() const requires (ROW_SIZE == COL_SIZE)
	{
		MatrixType<COL_SIZE, ROW_SIZE> result = {};
		for (size_t c = 0; c < COL_SIZE; c++)
		{
			for (size_t r = 0; r < ROW_SIZE; r++)
			{
				result.m_elements[c][r] == (c == r) ? 1 : 0;
			}
		}
		return result;
	}

	MatrixType operator+(const MatrixType<ROW_SIZE, COL_SIZE>& other) const
	{
		MatrixType result = {};
		for (size_t c = 0; c < COL_SIZE; c++)
		{
			for (size_t r = 0; r < ROW_SIZE; r++)
			{
				result.m_elements[c][r] = m_elements[c][r] + other[c][r];
			}
		}
		return result;
	}
	MatrixType& operator+=(const MatrixType<ROW_SIZE, COL_SIZE>& other)
	{
		for (size_t c = 0; c < COL_SIZE; c++)
		{
			for (size_t r = 0; r < ROW_SIZE; r++)
			{
				m_elements[c][r] += other[c][r];
			}
		}
		return *this;
	}

	MatrixType operator-(const MatrixType<ROW_SIZE, COL_SIZE>& other) const
	{
		MatrixType result = {};
		for (size_t c = 0; c < COL_SIZE; c++)
		{
			for (size_t r = 0; r < ROW_SIZE; r++)
			{
				result.m_elements[c][r] = m_elements[c][r] - other[c][r];
			}
		}
		return result;
	}
	MatrixType& operator-=(const MatrixType<ROW_SIZE, COL_SIZE>& other)
	{
		for (size_t c = 0; c < COL_SIZE; c++)
		{
			for (size_t r = 0; r < ROW_SIZE; r++)
			{
				m_elements[c][r] -= other[c][r];
			}
		}
		return *this;
	}

	MatrixType operator*(const float scalar) const
	{
		MatrixType result = {};
		for (size_t c = 0; c < COL_SIZE; c++)
		{
			for (size_t r = 0; r < ROW_SIZE; r++)
			{
				result.m_elements[c][r] = m_elements[c][r] * scalar;
			}
		}
		return result;
	}
	MatrixType& operator*=(const float scalar)
	{
		for (size_t c = 0; c < COL_SIZE; c++)
		{
			for (size_t r = 0; r < ROW_SIZE; r++)
			{
				m_elements[c][r] *= scalar;
			}
		}
		return *this;
	}

	template<size_t OTHER_ROW_SIZE, size_t OTHER_COL_SIZE>
	requires (COL_SIZE == OTHER_ROW_SIZE)
	MatrixType operator*(const MatrixType<OTHER_ROW_SIZE, OTHER_COL_SIZE>& other) const
	{
		MatrixType result = {};
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			for (size_t otherC = 0; otherC < OTHER_COL_SIZE; otherC++)
			{
				for (size_t c = 0; c < COL_SIZE; c++)
				{
					result.m_elements[otherC][r] += m_elements[c][r] * other.m_elements[otherC][c];
				}
			}
		}
		return result;
	}

	template<size_t VEC_SIZE>
	requires (VEC_SIZE == COL_SIZE && std::is_default_constructible_v<Vec<float, ROW_SIZE>>)
	Vec<float, ROW_SIZE> operator*(const Vec<float, VEC_SIZE>& vec) const
	{
		Vec<float, ROW_SIZE> result = {};
		for (size_t c = 0; c < COL_SIZE; c++)
		{
			for (size_t r = 0; r < ROW_SIZE; r++)
			{
				result[r] += m_elements[c][r] * vec[c];
			}
		}
		return result;
	}
};

using Mat4 = MatrixType<4, 4>;
using Mat3 = MatrixType<3, 3>;