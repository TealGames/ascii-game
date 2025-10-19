#pragma once
#include <array>
#include <cstdint>
#include <format>
#include <string>
#include "Utils/Data/Vec4Type.hpp"

//TODO: add other matrix operations like inversion, row swapping, gausian elimination, determinant

template<size_t ROW_SIZE, size_t COL_SIZE>
requires (ROW_SIZE >0 && COL_SIZE >0)
class MatrixType
{
private:
	/// <summary>
	/// Note: elements are stored in COLUMN MAJOR ORDER:
	/// this means [0][0]..[0][3] is FIRST COLUMN, [1][0]..[1][3] is SECOND COLUMN
	/// </summary>
	Vec<float, COL_SIZE> m_arr[ROW_SIZE];
public:
	static constexpr size_t MATRIX_SIZE = ROW_SIZE * COL_SIZE;

private:
public: 
	constexpr MatrixType() : m_arr() {}
	/// <summary>
	/// Constructs a matrix using a 2D array in ROW MAJOR order
	/// NOTE: this is the same as arr[ROW_SIZE][COL_SIZE] since arr
	/// pointer decays to float* [COL_SIZE] pointer -> pointer to one row
	/// </summary>
	/// <param name="rowMajorElements"></param>
	constexpr MatrixType(const float (*rowMajorElements)[COL_SIZE]) : m_arr() 
	{
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			for (size_t c = 0; c < COL_SIZE; c++)
			{
				m_arr[c][r] = rowMajorElements[r][c];
			}
		}
	}
	/// <summary>
	/// Constructs a matrix using C++ built in array type in ROW MAJOR order
	/// </summary>
	/// <param name="rowMajorElements"></param>
	constexpr MatrixType(const std::array<std::array<float, COL_SIZE>, ROW_SIZE>& rowMajorElements) : m_arr()
	{
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			for (size_t c = 0; c < COL_SIZE; c++)
			{
				m_arr[c][r] = rowMajorElements[r][c];
			}
		}
	}
	/// <summary>
	/// Constructs a matrix using an initial first element pointer.
	/// NOTE: memory must be laid out in ROW MAJOR order and total floats must be equal to ROW_SIZE * COL_SIZE
	/// 
	/// </summary>
	/// <param name="firstElementPtr"></param>
	constexpr MatrixType(const float* firstElementPtr)
	{
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			for (size_t c = 0; c < COL_SIZE; c++)
			{
				m_arr[c][r] = *(firstElementPtr + (COL_SIZE * r) + c);
			}
		}
	}

	static constexpr MatrixType GetIdentity() requires (ROW_SIZE == COL_SIZE)
	{
		MatrixType<ROW_SIZE, COL_SIZE> result = {};
		for (size_t c = 0; c < COL_SIZE; c++)
		{
			for (size_t r = 0; r < ROW_SIZE; r++)
			{
				result.m_arr[c][r] = (c == r) ? 1 : 0;
			}
		}
		return result;
	}

	bool IsZero() const
	{
		for (size_t c = 0; c < COL_SIZE; c++)
		{
			for (size_t r = 0; r < ROW_SIZE; r++)
			{
				if (!Utils::ApproximateEqualsF(m_arr[r][c], 0.0f))
					return false;
			}
		}
		return true;
	}

	const float* GetMemPointer() const
	{
		return &m_arr[0][0];
	}

	std::array<float, MATRIX_SIZE> GetElementsRowMajor() const
	{
		std::array<float, MATRIX_SIZE> arr = {};
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			for (size_t c = 0; c < COL_SIZE; c++)
			{
				arr[r * COL_SIZE + c] = m_arr[c][r];
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
				arr[c * COL_SIZE + r] = m_arr[c][r];
			}
		}
		return arr;
	}

	std::array<std::array<float, COL_SIZE>, ROW_SIZE> GetElements2DRowMajor() const
	{
		std::array<std::array<float, COL_SIZE>, ROW_SIZE> arr = {};
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			for (size_t c = 0; c < COL_SIZE; c++)
			{
				arr[r][c] = m_arr[c][r];
			}
		}
		return arr;
	}

	std::array<std::array<float, ROW_SIZE>, COL_SIZE> GetElements2DColMajor() const
	{
		return m_arr;
	}

	float Get(const std::uint8_t r, const std::uint8_t c) const
	{
		if (r >= ROW_SIZE || c >= COL_SIZE)
		{
			throw std::invalid_argument(std::format("Invalid matrix[{}][{}] get access:({}, {})", ROW_SIZE, COL_SIZE, r, c));
			return 0;
		}
		return m_arr[c][r];
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
			result[c] = m_arr[c][r];
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
			result[c] = m_arr[c][r];
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
			result[r] = m_arr[c][r];
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
		return m_arr[c];
	}
	
	/// <summary>
	/// Will get the top leftmost slice based on size arguments from this matrix
	/// </summary>
	/// <typeparam name="SLICE_ROW_SIZE"></typeparam>
	/// <typeparam name="SLICE_COL_SIZE"></typeparam>
	/// <returns></returns>
	template<size_t SLICE_ROW_SIZE, size_t SLICE_COL_SIZE>
	requires (SLICE_ROW_SIZE <= ROW_SIZE && SLICE_COL_SIZE <= COL_SIZE)
	MatrixType<SLICE_ROW_SIZE, SLICE_COL_SIZE> GetSlice() const
	{
		std::array<std::array<float, SLICE_COL_SIZE>, SLICE_ROW_SIZE> slicedArr = {};
		for (size_t r = 0; r< SLICE_ROW_SIZE; r++)
		{
			for (size_t c = 0; c < SLICE_COL_SIZE; c++)
			{
				slicedArr[r][c] = m_arr[c][r];
			}
		}
		return MatrixType<SLICE_ROW_SIZE, SLICE_COL_SIZE>(slicedArr);
	}


	void Set(const std::uint8_t r, const std::uint8_t c, const float newVal)
	{
		if (r >= ROW_SIZE || c >= COL_SIZE)
		{
			throw std::invalid_argument(std::format("Invalid matrix[{}][{}] set access:({}, {})", ROW_SIZE, COL_SIZE, r, c));
			return;
		}
		m_arr[c][r] = newVal;
	}
	void Set(const std::uint8_t c, const std::array<float, ROW_SIZE>& vals)
	{
		if (c >= COL_SIZE)
		{
			throw std::invalid_argument(std::format("Invalid matrix[{}][{}] col set access:{}", ROW_SIZE, COL_SIZE, c));
			return;
		}
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			m_arr[c][r] = vals[r];
		}
	}
	void Set(const std::uint8_t c, const Vec<float, ROW_SIZE>& vals)
	{
		if (c >= COL_SIZE)
		{
			throw std::invalid_argument(std::format("Invalid matrix[{}][{}] col set access:{}", ROW_SIZE, COL_SIZE, c));
			return;
		}
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			m_arr[c][r] = vals[r];
		}
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
				result.m_arr[r][c] = m_arr[c][r];
			}
		}
		return result;
	}

	bool Inverse(MatrixType<ROW_SIZE, COL_SIZE>* outMatrix) const requires (ROW_SIZE == 2 && COL_SIZE == 2)
	{
		const float determinant = m_arr[0][0] * m_arr[1][1] - m_arr[0][1] * m_arr[1][0];
		if (Utils::ApproximateEqualsF(determinant, 0.0f))
			return false;

		if (outMatrix != nullptr)
		{
			const float inverseDeterminant = 1 / determinant;
			*outMatrix = MatrixType<ROW_SIZE, COL_SIZE>(std::array<std::array<float, COL_SIZE>, ROW_SIZE>
			{{
				{{m_arr[1][1] * inverseDeterminant, -m_arr[1][0] * inverseDeterminant }},
				{{-m_arr[0][1] * inverseDeterminant, m_arr[0][0] * inverseDeterminant }}
			}});
		}
		return true;
	}
	bool Inverse(MatrixType<ROW_SIZE, COL_SIZE>* outMatrix) const requires (ROW_SIZE == 3 && COL_SIZE == 3)
	{
		const float determinant =
			m_arr[0][0] * (m_arr[1][1] * m_arr[2][2] - m_arr[1][2] * m_arr[2][1])
			- m_arr[0][1] * (m_arr[1][0] * m_arr[2][2] - m_arr[1][2] * m_arr[2][0])
			+ m_arr[0][2] * (m_arr[1][0] * m_arr[2][1] - m_arr[1][1] * m_arr[2][0]);

		if (Utils::ApproximateEqualsF(determinant, 0.0f))
			return false;

		if (outMatrix != nullptr)
		{
			const float inverseDeterminant = 1 / determinant;
			//While we could use create matrix out of cofactors and transpose, we get adjugate (transpose of cofactors)
			//directy so we skip creating an intermediate matrix
			*outMatrix = MatrixType<ROW_SIZE, COL_SIZE>(std::array<std::array<float, COL_SIZE>, ROW_SIZE>
			{{
				{{ 
					(m_arr[1][1] * m_arr[2][2] - m_arr[1][2] * m_arr[2][1]) * inverseDeterminant,
					-(m_arr[1][0] * m_arr[2][2] - m_arr[1][2] * m_arr[2][0]) * inverseDeterminant,
					(m_arr[1][0] * m_arr[2][1] - m_arr[1][1] * m_arr[2][0]) * inverseDeterminant
				}},
				{{
					-(m_arr[0][1] * m_arr[2][2] - m_arr[0][2] * m_arr[2][1]) * inverseDeterminant,
					(m_arr[0][0] * m_arr[2][2] - m_arr[0][2] * m_arr[2][0]) * inverseDeterminant,
					-(m_arr[0][0] * m_arr[2][1] - m_arr[0][1] * m_arr[2][0]) * inverseDeterminant
				}},
				{{
					(m_arr[0][1] * m_arr[1][2] - m_arr[0][2] * m_arr[1][1]) * inverseDeterminant,
					-(m_arr[0][0] * m_arr[1][2] - m_arr[0][2] * m_arr[1][0]) * inverseDeterminant,
					(m_arr[0][0] * m_arr[1][1] - m_arr[0][1] * m_arr[1][0]) * inverseDeterminant
				}}
			}});
		}
		return true;
	}
	bool Inverse(MatrixType<ROW_SIZE, COL_SIZE>* outMatrix) const requires (ROW_SIZE == 4 && COL_SIZE == 4)
	{
		//Top left (0..1, rows 0..1) minors
		const float s0 = m_arr[0][0] * m_arr[1][1] - m_arr[0][1] * m_arr[1][0];
		const float s1 = m_arr[0][0] * m_arr[2][1] - m_arr[0][1] * m_arr[2][0];
		const float s2 = m_arr[0][0] * m_arr[3][1] - m_arr[0][1] * m_arr[3][0];
		const float s3 = m_arr[1][0] * m_arr[2][1] - m_arr[1][1] * m_arr[2][0];
		const float s4 = m_arr[1][0] * m_arr[3][1] - m_arr[1][1] * m_arr[3][0];
		const float s5 = m_arr[2][0] * m_arr[3][1] - m_arr[2][1] * m_arr[3][0];

		//Bottom right (cols 2..3, rows 2..3) minors
		const float c0 = m_arr[0][2] * m_arr[1][3] - m_arr[0][3] * m_arr[1][2];
		const float c1 = m_arr[0][2] * m_arr[2][3] - m_arr[0][3] * m_arr[2][2];
		const float c2 = m_arr[0][2] * m_arr[3][3] - m_arr[0][3] * m_arr[3][2];
		const float c3 = m_arr[1][2] * m_arr[2][3] - m_arr[1][3] * m_arr[2][2];
		const float c4 = m_arr[1][2] * m_arr[3][3] - m_arr[1][3] * m_arr[3][2];
		const float c5 = m_arr[2][2] * m_arr[3][3] - m_arr[2][3] * m_arr[3][2];

		const float determinant = s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0;
		if (Utils::ApproximateEqualsF(determinant, 0.0f))
			return false;

		if (outMatrix != nullptr)
		{
			const float inverseDeterminant = 1 / determinant;

			//NOTE: the output here is in row major order since we create matrices via row major (but internally store as col major)
			//so the transpose automatically happens in constructor
			*outMatrix = MatrixType<ROW_SIZE, COL_SIZE>(std::array<std::array<float, COL_SIZE>, ROW_SIZE>
			{{
				{{
					(m_arr[1][1] * c5 - m_arr[2][1] * c4 + m_arr[3][1] * c3)* inverseDeterminant,
					-(m_arr[1][0] * c5 - m_arr[2][0] * c4 + m_arr[3][0] * c3) * inverseDeterminant,
					(m_arr[0][0] * c5 - m_arr[2][0] * c2 + m_arr[3][0] * c1)* inverseDeterminant,
					-(m_arr[0][0] * c4 - m_arr[1][0] * c2 + m_arr[3][0] * c0) * inverseDeterminant
				}},

				{{
					-(m_arr[1][1] * c4 - m_arr[2][1] * c3 + m_arr[3][1] * c2) * inverseDeterminant,
					(m_arr[1][0] * c4 - m_arr[2][0] * c3 + m_arr[3][0] * c2)* inverseDeterminant,
					-(m_arr[0][0] * c4 - m_arr[2][0] * c1 + m_arr[3][0] * c0) * inverseDeterminant,
					(m_arr[0][0] * c3 - m_arr[1][0] * c1 + m_arr[3][0] * c0)* inverseDeterminant
				}},

				{{
					(m_arr[1][1] * s5 - m_arr[2][1] * s4 + m_arr[3][1] * s3)* inverseDeterminant,
					-(m_arr[1][0] * s5 - m_arr[2][0] * s4 + m_arr[3][0] * s3) * inverseDeterminant,
					(m_arr[0][0] * s5 - m_arr[2][0] * s2 + m_arr[3][0] * s1)* inverseDeterminant,
					-(m_arr[0][0] * s4 - m_arr[1][0] * s2 + m_arr[3][0] * s0) * inverseDeterminant
				}},

				{{
					-(m_arr[1][1] * s4 - m_arr[2][1] * s3 + m_arr[3][1] * s2) * inverseDeterminant,
					(m_arr[1][0] * s4 - m_arr[2][0] * s3 + m_arr[3][0] * s2)* inverseDeterminant,
					-(m_arr[0][0] * s4 - m_arr[2][0] * s1 + m_arr[3][0] * s0) * inverseDeterminant,
					(m_arr[0][0] * s3 - m_arr[1][0] * s1 + m_arr[2][0] * s0)* inverseDeterminant
				}}
				 
			}});
		}
		return true;
	}

	MatrixType operator+(const MatrixType<ROW_SIZE, COL_SIZE>& other) const
	{
		MatrixType result = {};
		for (size_t c = 0; c < COL_SIZE; c++)
		{
			for (size_t r = 0; r < ROW_SIZE; r++)
			{
				result.m_arr[c][r] = m_arr[c][r] + other[c][r];
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
				m_arr[c][r] += other[c][r];
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
				result.m_arr[c][r] = m_arr[c][r] - other[c][r];
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
				m_arr[c][r] -= other[c][r];
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
				result.m_arr[c][r] = m_arr[c][r] * scalar;
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
				m_arr[c][r] *= scalar;
			}
		}
		return *this;
	}

	template<size_t OTHER_ROW_SIZE, size_t OTHER_COL_SIZE>
	requires (COL_SIZE == OTHER_ROW_SIZE)
	MatrixType<ROW_SIZE, OTHER_COL_SIZE> operator*(const MatrixType<OTHER_ROW_SIZE, OTHER_COL_SIZE>& other) const
	{
		MatrixType<ROW_SIZE, OTHER_COL_SIZE> result = {};
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			for (size_t otherC = 0; otherC < OTHER_COL_SIZE; otherC++)
			{
				for (size_t c = 0; c < COL_SIZE; c++)
				{
					result.m_arr[otherC][r] += m_arr[c][r] * other.m_arr[otherC][c];
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
				result[r] += m_arr[c][r] * vec[c];
			}
		}
		return result;
	}

	std::string ToString(const bool newLineOnRow=true) const
	{
		std::string result = "";
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			if (newLineOnRow) result += '\n';
			result += '[' + std::to_string(m_arr[0][r]);
			for (size_t c = 1; c < COL_SIZE; c++)
			{
				result += ',' + std::to_string(m_arr[c][r]);
			}
			result += ']';
		}
		if (newLineOnRow) result += '\n';
		return result;
	}
};

using Mat4 = MatrixType<4, 4>;
using Mat3 = MatrixType<3, 3>;