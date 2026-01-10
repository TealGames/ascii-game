#pragma once
#include <array>
#include <cstdint>
#include <format>
#include <string>
#include "Utils/Data/Vec4Type.hpp"

constexpr float DETERMINANT_EPSILON = 1e-12;

enum class MatrixMajorOrder : std::uint8_t
{
	Column	= 0,
	Row		= 1,
};

template <typename T, size_t ROW_SIZE, size_t COL_SIZE, MatrixMajorOrder ORDER, size_t ORDER_ALIGN_BYTES>
struct MatrixStorage {};

template<typename T, size_t SIZE, size_t ORDER_ALIGN_BYTES>
using MatrixMemoryRow = Vec<T, SIZE, ORDER_ALIGN_BYTES>;

template <typename T, size_t ROW_SIZE, size_t COL_SIZE, size_t ORDER_ALIGN_BYTES>
struct MatrixStorage<T, ROW_SIZE, COL_SIZE, MatrixMajorOrder::Row, ORDER_ALIGN_BYTES>
{
	// For ROW major order, each ROW is stored contiguously 
	// So a mat2x4 (2 rows, 4 cols)
	// | 1 2 3 4 |
	// | 5 6 7 8 |
	// will be stored as: [1, 2, 3, 4] [5, 6, 7, 8]
	using Type = MatrixMemoryRow<T, COL_SIZE, ORDER_ALIGN_BYTES>[ROW_SIZE];
};

template<typename T, size_t ROW_SIZE, size_t COL_SIZE, size_t ORDER_ALIGN_BYTES>
struct MatrixStorage<T, ROW_SIZE, COL_SIZE, MatrixMajorOrder::Column, ORDER_ALIGN_BYTES>
{
	// For COL major order, each COLUMN is stored contiguously 
	// So a mat2x4 (2 rows, 4 cols)
	// | 1 2 3 4 |
	// | 5 6 7 8 |
	// will be stored as: [1, 5] [2, 6] [3, 7] [4, 8]
	using Type = MatrixMemoryRow<T, ROW_SIZE, ORDER_ALIGN_BYTES>[COL_SIZE];
};

template<size_t ROW_SIZE, size_t COL_SIZE>
concept IsPositiveSize = (ROW_SIZE > 0 && COL_SIZE > 0);

//TODO: add other matrix operations like inversion, row swapping, gausian elimination, determinant

template<typename T, size_t ROW_SIZE, size_t COL_SIZE, MatrixMajorOrder ORDER, size_t ORDER_ALIGN_BYTES = 0>
requires (IsPositiveSize<ROW_SIZE, COL_SIZE> && std::is_arithmetic_v<T>)
/// <summary>
/// A matrix data type defined by its row size, col size, internal storage ORDER and the ORDER ALIGNMENT (in bytes)
/// NOTE: alignment changes depending on ORDER. If ORDER is ROW -> alignment for ROWS, ORDER is COL -> alignment for COLS
/// Default alignment is 0 -> natural alignment
/// 
/// VERY IMPORTANT:
/// -> STORAGE ORDER(template arg ORDER) :
///	   The matrix INTERNALLY can store the data in COL major OR ROW major order regardless of API
///	   with the intention of matching any other API order, such as col major storage for OpenGL for example
/// 
/// -> API INDEXING: 
///	   Elements are accessed via Get(row, col) / Set(row, col) using standard matrix coordinates.
///    This is universal notation and does NOT imply a mathematical convention. This can be viewed as 
///	   simply accessing a chunk of data by row and col with the order of the data set by the user.
/// 
/// -> MATH CONVENTION:
///	   The supported math convention is COL MAJOR meaning vector-matrix multiplication is done in COLUMNS
///	   and in the order MATRIX * VECTOR. This also means COL 0 is X-basis vector, COL 1 is Y-basis vector, 
///	   COL 2 is Z-basis vector.
/// 
/// -> CONSTRUCTOR INPUT FORMAT:
///	   Constructors accept data in "row-major format" (array of rows) for convenience:
///    Mat4({{
///       {1, 0, 0, Tx},  // This becomes: X.x, Y.x, Z.x, T.x
///       {0, 1, 0, Ty},  // This becomes: X.y, Y.y, Z.y, T.y
///       {0, 0, 1, Tz},  // This becomes: X.z, Y.z, Z.z, T.z
///       {0, 0, 0, 1}    // This becomes: X.w, Y.w, Z.w, T.w
///    }})
///    Due to column-major convention math convention, each "row" you input becomes a horizontal slice
///    across all column vectors (NOT a row vector itself)
/// 
/// SUMMARY:
/// - Storage order (memory layout) does not affect mathematical operations
/// - Use Get/SetCol for semantic operations (basis vectors, translation)
/// - Supply transformation data with vectors in COLUMNS, translation in column 3
/// </summary>
class MatrixType
{
private:
	MatrixStorage<T,ROW_SIZE, COL_SIZE, ORDER, ORDER_ALIGN_BYTES>::Type m_arr;
public:
	static constexpr size_t MATRIX_SIZE = ROW_SIZE * COL_SIZE;

private:
	T CalculateDeterminant(std::uint8_t r0, std::uint8_t c0, std::uint8_t r1, std::uint8_t c1, 
						   std::uint8_t r2, std::uint8_t c2, std::uint8_t r3, std::uint8_t c3) const
	{
		/* determinant of:
		*  | 0 1 |
		*  | 2 3 |
		*  is: 0 * 3 - 1 * 2
		*/
		return GetUnsafe(r0, c0) * GetUnsafe(r3, c3) - GetUnsafe(r1, c1) * GetUnsafe(r2, c2);
	}
public: 
	constexpr MatrixType() : m_arr() {}
	/// <summary>
	/// Constructs a matrix using a 2D array in ROW MAJOR order
	/// NOTE: this is the same as arr[ROW_SIZE][COL_SIZE] since arr
	/// pointer decays to float* [COL_SIZE] pointer -> pointer to one row
	/// </summary>
	/// <param name="rowMajorElements"></param>
	constexpr MatrixType(const T (*rowMajorElements)[COL_SIZE]) : m_arr() 
	{
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			for (size_t c = 0; c < COL_SIZE; c++)
			{
				SetUnsafe(r, c, rowMajorElements[r][c]);
			}
		}
	}

	/// <summary>
	/// Constructs a matrix built in array type in ROW MAJOR order
	/// </summary>
	/// <param name="rowMajorElements"></param>
	constexpr MatrixType(const std::array<std::array<T, COL_SIZE>, ROW_SIZE>& rowMajorElements) : m_arr()
	{
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			for (size_t c = 0; c < COL_SIZE; c++)
			{
				SetUnsafe(r, c, rowMajorElements[r][c]);
			}
		}
	}
	
	constexpr MatrixType(const std::array<Vec<T, COL_SIZE>, ROW_SIZE>& rowMajorElements) : m_arr()
	{
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			for (size_t c = 0; c < COL_SIZE; c++)
			{
				SetUnsafe(r, c, rowMajorElements[r][c]);
			}
		}
	}

	/// <summary>
	/// Constructs a matrix from a smaller matrix and fills the gaps with 0
	/// </summary>
	/// <typeparam name="OTHER_ROW_SIZE"></typeparam>
	/// <typeparam name="OTHER_COL_SIZE"></typeparam>
	template<size_t OTHER_ROW_SIZE, size_t OTHER_COL_SIZE, MatrixMajorOrder OTHER_ORDER, size_t OTHER_ALIGN>
	requires (IsPositiveSize<OTHER_ROW_SIZE, OTHER_COL_SIZE> && OTHER_ROW_SIZE <= ROW_SIZE && OTHER_COL_SIZE <= COL_SIZE)
	constexpr MatrixType(const MatrixType<T,OTHER_ROW_SIZE, OTHER_COL_SIZE, OTHER_ORDER, OTHER_ALIGN>& other) : m_arr()
	{
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			for (size_t c = 0; c < COL_SIZE; c++)
			{
				if (r < OTHER_ROW_SIZE && c < OTHER_COL_SIZE)
					SetUnsafe(r, c, other.Get(r, c));
				else SetUnsafe(r,c, 0);
			}
		}
	}
	/// <summary>
	/// Constructs a matrix using an initial first element pointer.
	/// NOTE: memory must be laid out in ROW MAJOR order and total floats must be equal to ROW_SIZE * COL_SIZE
	/// 
	/// </summary>
	/// <param name="firstElementPtr"></param>
	constexpr MatrixType(const T* firstElementPtr) : m_arr()
	{
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			for (size_t c = 0; c < COL_SIZE; c++)
			{
				SetUnsafe(r, c, *(firstElementPtr + (COL_SIZE * r) + c));
			}
		}
	}

	static constexpr size_t GetRowSize() { return ROW_SIZE; }
	static constexpr size_t GetRowSizeBytes() { return sizeof(T) * ROW_SIZE; }
	static constexpr size_t GetColSize() { return COL_SIZE; }
	static constexpr size_t GetColSizeBytes() { return sizeof(T) * COL_SIZE; }

	/// <summary>
	/// Will return the extra padding based on the ORDER specified.
	/// -> ROW: the extra padding in BYTES per row
	/// -> COL: the extra padding in BYTES per col
	/// </summary>
	/// <returns></returns>
	static constexpr size_t GetExtraPaddingBytes()
	{
		if constexpr (ORDER_ALIGN_BYTES == 0)
			return 0;

		if constexpr (ORDER == MatrixMajorOrder::Row)
			return GetSingleMemoryRowSizeBytes() - GetRowSizeBytes();
		else
			return GetSingleMemoryRowSizeBytes() - GetColSizeBytes();
	}
	/// <summary>
	/// Will return the total memory size of one MEMORY row (NOT the same as matrix row)
	/// -> ROW: total BYTE size of one ROW INCLUDING PADDING from alignment
	/// -> COL: total BYTE size of one COL INCLUDING PADDING from alignment
	/// </summary>
	/// <returns></returns>
	static constexpr size_t GetSingleMemoryRowSizeBytes()
	{
		if constexpr (ORDER == MatrixMajorOrder::Row)
			return sizeof(MatrixMemoryRow<T, ROW_SIZE, ORDER_ALIGN_BYTES>);
		else return sizeof(MatrixMemoryRow<T, COL_SIZE, ORDER_ALIGN_BYTES>);
	}

	static constexpr MatrixType GetIdentity() requires (ROW_SIZE == COL_SIZE)
	{
		MatrixType result = {};
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			for (size_t c = 0; c < COL_SIZE; c++)
			{
				result.SetUnsafe(r, c, (c == r) ? 1 : 0);
			}
		}
		return result;
	}

	bool IsAllZero() const
	{
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			for (size_t c = 0; c < COL_SIZE; c++)
			{
				if (!Utils::ApproximateEqualsF(GetUnsafe(r, c), 0.0f))
					return false;
			}
		}
		return true;
	}

	bool HasValue(const T& val) const
	{
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			for (size_t c = 0; c < COL_SIZE; c++)
			{
				if (Utils::ApproximateEqualsF(GetUnsafe(r, c), val))
					return true;
			}
		}
		return false;
	}
	bool HasNegativeValues() const
	{
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			for (size_t c = 0; c < COL_SIZE; c++)
			{
				if (GetUnsafe(r, c) < 0)
					return true;
			}
		}
		return false;
	}

	/// <summary>
	/// A matrix is orthonormal if 3 conditions hold:
	/// 1) Each basis axis is a unit vector (has magnitude 1)
	/// 2) All basis axes are perpendicular to one another (dot product 0)
	/// 3) Handedness is consistent (Right Handed -> determinant is always around 1, Left Handed -> -1)
	/// </summary>
	/// <returns></returns>
	bool IsOrthonormal() const requires (ROW_SIZE == 3 && COL_SIZE == 3)
	{
		const Vec<T, ROW_SIZE> basisX = GetBasisVectorX();
		if (!basisX.IsUnitVector())
			return false;

		const Vec<T, ROW_SIZE> basisY = GetBasisVectorY();
		if (!basisY.IsUnitVector())
			return false;

		const Vec<T, ROW_SIZE> basisZ = GetBasisVectorZ();
		if (!basisZ.IsUnitVector())
			return false;

		if (!Utils::ApproximateEqualsF(DotProduct(basisX, basisY), 0) || 
			!Utils::ApproximateEqualsF(DotProduct(basisY, basisZ), 0) ||
			!Utils::ApproximateEqualsF(DotProduct(basisX, basisZ), 0))
		{
			return false;
		}

		return Utils::ApproximateEqualsF(CalculateDeterminant(), 1);
	}

	const T* GetMemPointer() const { return &m_arr[0][0]; }

#pragma region GetFunctions
	// --------------------------------------------------------------------------------------------------------
	//										GET FUNCTIONS
	// --------------------------------------------------------------------------------------------------------

	Vec<T, ROW_SIZE> GetBasisVectorX() const { return GetColVector(0); }
	Vec<T, ROW_SIZE> GetBasisVectorY() const { return GetColVector(1); }
	Vec<T, ROW_SIZE> GetBasisVectorZ() const { return GetColVector(2); }

	std::array<T, MATRIX_SIZE> GetElementsRowMajor() const
	{
		std::array<T, MATRIX_SIZE> arr = {};
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			for (size_t c = 0; c < COL_SIZE; c++)
			{
				arr[r * COL_SIZE + c] = GetUnsafe(r, c);
			}
		}
		return arr;
	}
	std::array<T, MATRIX_SIZE> GetElementsColMajor() const
	{
		std::array<T, MATRIX_SIZE> arr = {};
		for (size_t c = 0; c < COL_SIZE; c++)
		{
			for (size_t r = 0; r < ROW_SIZE; r++)
			{
				arr[c * ROW_SIZE + r] = GetUnsafe(r, c);
			}
		}
		return arr;
	}

	std::array<std::array<T, COL_SIZE>, ROW_SIZE> GetElements2DRowMajor() const
	{
		std::array<std::array<T, COL_SIZE>, ROW_SIZE> arr = {};
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			for (size_t c = 0; c < COL_SIZE; c++)
			{
				arr[r][c] = GetUnsafe(r, c);
			}
		}
		return arr;
	}

	std::array<std::array<T, ROW_SIZE>, COL_SIZE> GetElements2DColMajor() const
	{
		std::array<std::array<T, ROW_SIZE>, COL_SIZE> arr = {};
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			for (size_t c = 0; c < COL_SIZE; c++)
			{
				arr[c][r] = GetUnsafe(r, c);
			}
		}
		return arr;
	}

	constexpr const T& GetUnsafe(const std::uint8_t r, const std::uint8_t c) const
	{
		if constexpr (ORDER == MatrixMajorOrder::Row) return m_arr[r][c];
		else return m_arr[c][r];
	}

	constexpr T& GetMutableUnsafe(const std::uint8_t r, const std::uint8_t c)
	{
		if constexpr (ORDER == MatrixMajorOrder::Row) return m_arr[r][c];
		else return m_arr[c][r];
	}

	const T& Get(const std::uint8_t r, const std::uint8_t c) const
	{
		ENGINE_ASSERT(r < ROW_SIZE && c < COL_SIZE, "Invalid matrix[{}][{}] get access:({}, {})", ROW_SIZE, COL_SIZE, r, c);
		return GetUnsafe(r, c);
	}

	T& GetMutable(const std::uint8_t r, const std::uint8_t c)
	{
		ENGINE_ASSERT(r < ROW_SIZE && c < COL_SIZE, "Invalid matrix[{}][{}] get (mutable) access:({}, {})", ROW_SIZE, COL_SIZE, r, c);
		return GetMutableUnsafe(r, c);
	}

	std::array<T, COL_SIZE> GetRowArray(const std::uint8_t r) const
	{
		ENGINE_ASSERT(r < ROW_SIZE, "Invalid matrix[{}][{}] get row array access:{}", ROW_SIZE, COL_SIZE, r);
		std::array<T, COL_SIZE> result = {};
		for (size_t c = 0; c < COL_SIZE; c++)
		{
			result[c] = GetUnsafe(r, c);
		}
		return result;
	}
	Vec<T, COL_SIZE> GetRowVector(const std::uint8_t r) const
	{
		ENGINE_ASSERT(r < ROW_SIZE, "Invalid matrix[{}][{}] get row vector access:{}", ROW_SIZE, COL_SIZE, r);
		Vec<T, COL_SIZE> result = {};
		for (size_t c = 0; c < COL_SIZE; c++)
		{
			result[c] = GetUnsafe(r, c);
		}
		return result;
	}
	std::array<T, ROW_SIZE> GetColArray(const std::uint8_t c) const
	{
		ENGINE_ASSERT(c < COL_SIZE, "Invalid matrix[{}][{}] get col array access:{}", ROW_SIZE, COL_SIZE, c);
		std::array<T, ROW_SIZE> result = {};
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			result[r] = GetUnsafe(r, c);
		}
		return result;
	}
	Vec<T, ROW_SIZE> GetColVector(const std::uint8_t c) const
	{
		ENGINE_ASSERT(c < COL_SIZE, "Invalid matrix[{}][{}] get col vector access:{}", ROW_SIZE, COL_SIZE, c);
		Vec<T, ROW_SIZE> result = {};
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			result[r] = GetUnsafe(r, c);
		}
		return result;
	}
	
	/// <summary>
	/// Will get the top leftmost slice based on size arguments from this matrix
	/// </summary>
	/// <typeparam name="SLICE_ROW_SIZE"></typeparam>
	/// <typeparam name="SLICE_COL_SIZE"></typeparam>
	/// <returns></returns>
	template<size_t SLICE_ROW_SIZE, size_t SLICE_COL_SIZE>
	requires (IsPositiveSize<SLICE_ROW_SIZE, SLICE_COL_SIZE> 
			  && SLICE_ROW_SIZE <= ROW_SIZE && SLICE_COL_SIZE <= COL_SIZE)
		MatrixType<T,SLICE_ROW_SIZE, SLICE_COL_SIZE, ORDER> GetSlice() const
	{
		std::array<std::array<T, SLICE_COL_SIZE>, SLICE_ROW_SIZE> slicedArr = {};
		for (size_t r = 0; r < SLICE_ROW_SIZE; r++)
		{
			for (size_t c = 0; c < SLICE_COL_SIZE; c++)
			{
				slicedArr[r][c] = GetUnsafe(r, c);
			}
		}
		return MatrixType<T, SLICE_ROW_SIZE, SLICE_COL_SIZE, ORDER>(slicedArr);
	}
#pragma endregion

	T CalculateDeterminant() const 
		requires (ROW_SIZE == 2 && COL_SIZE == 2)
	{
		return GetUnsafe(0, 0) * GetUnsafe(1, 1) - GetUnsafe(0, 1)* GetUnsafe(1, 0);
	}

	T CalculateDeterminant() const
		requires (ROW_SIZE == 3 && COL_SIZE == 3)
	{
		/* determinant of:
		*      0 1 2
		*  0 | a b c |
		*  1 | d e f |
		*  2 | g h i |
		*  is: 
		* 
		*   | e f |     | d f |     | d e |
		*  a| h i | - b | g i | + c | g h |
		*/
		return GetUnsafe(0, 0) * (GetUnsafe(1, 1) * GetUnsafe(2, 2) - GetUnsafe(1, 2) * GetUnsafe(2, 1)) -
			   GetUnsafe(0, 1) * (GetUnsafe(1, 0) * GetUnsafe(2, 2) - GetUnsafe(1, 2) * GetUnsafe(2, 0)) +
			   GetUnsafe(0, 2) * (GetUnsafe(1, 0) * GetUnsafe(2, 1) - GetUnsafe(1, 1) * GetUnsafe(2, 0));
	}

	T CalculateDeterminant() const
		requires (ROW_SIZE == 4 && COL_SIZE == 4)
	{
		T a = GetUnsafe(0, 0), b = GetUnsafe(0, 1), c = GetUnsafe(0, 2), d = GetUnsafe(0, 3);
		T e = GetUnsafe(1, 0), f = GetUnsafe(1, 1), g = GetUnsafe(1, 2), h = GetUnsafe(1, 3);
		T i = GetUnsafe(2, 0), j = GetUnsafe(2, 1), k = GetUnsafe(2, 2), l = GetUnsafe(2, 3);
		T m = GetUnsafe(3, 0), n = GetUnsafe(3, 1), o = GetUnsafe(3, 2), p = GetUnsafe(3, 3);

		return
			a * (f * (k * p - l * o) - g * (j * p - l * n) + h * (j * o - k * n)) -
			b * (e * (k * p - l * o) - g * (i * p - l * m) + h * (i * o - k * m)) +
			c * (e * (j * p - l * n) - f * (i * p - l * m) + h * (i * n - j * m)) -
			d * (e * (j * o - k * n) - f * (i * o - k * m) + g * (i * n - j * m));
	}

#pragma region SetFunctions
	// --------------------------------------------------------------------------------------------------------
	//										SET FUNCTIONS
	// --------------------------------------------------------------------------------------------------------

	constexpr void SetUnsafe(const std::uint8_t r, const std::uint8_t c, const T& newVal)
	{
		if constexpr (ORDER == MatrixMajorOrder::Row) m_arr[r][c] = newVal;
		else m_arr[c][r] = newVal;
	}

	void Set(const std::uint8_t r, const std::uint8_t c, const T& newVal)
	{
		ENGINE_ASSERT(r < ROW_SIZE && c < COL_SIZE, "Invalid matrix[{}][{}] set access:({}, {})", ROW_SIZE, COL_SIZE, r, c);
		SetUnsafe(r, c, newVal);
	}
	void SetCol(const std::uint8_t c, const std::array<T, ROW_SIZE>& vals)
	{
		ENGINE_ASSERT(c < COL_SIZE, "Invalid matrix[{}][{}] col set access:{}", ROW_SIZE, COL_SIZE, c);
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			SetUnsafe(r, c, vals[r]);
		}
	}

	template<size_t OTHER_ROW_SIZE>
	requires (OTHER_ROW_SIZE >= 2 && OTHER_ROW_SIZE <= ROW_SIZE)
	void SetCol(const std::uint8_t c, const Vec<T, OTHER_ROW_SIZE>&vals)
	{
		ENGINE_ASSERT(c < COL_SIZE, "Invalid matrix[{}][{}] col set access:{}", ROW_SIZE, COL_SIZE, c);
		for (size_t r = 0; r < OTHER_ROW_SIZE; r++)
		{
			SetUnsafe(r, c, vals[r]);
		}
	}

	void SetRow(const std::uint8_t r, const std::array<T, COL_SIZE>& vals)
	{
		ENGINE_ASSERT(r < ROW_SIZE, "Invalid matrix[{}][{}] row set access:{}", ROW_SIZE, COL_SIZE, r);
		for (size_t c = 0; c < COL_SIZE; c++)
		{
			SetUnsafe(r, c, vals[c]);
		}
	}
	void SetRow(const std::uint8_t r, const Vec<T, COL_SIZE>& vals)
	{
		ENGINE_ASSERT(r < ROW_SIZE, "Invalid matrix[{}][{}] row set access:{}", ROW_SIZE, COL_SIZE, r);
		for (size_t c = 0; c < COL_SIZE; c++)
		{
			SetUnsafe(r, c, vals[c]);
		}
	}
	void SetTopLeft(const std::array<std::array<T, COL_SIZE>, ROW_SIZE>& rowMajorElements)
	{
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			for (size_t c = 0; c < COL_SIZE; c++)
			{
				SetUnsafe(r, c, rowMajorElements[r][c]);
			}
		}
	}
	template<size_t OTHER_ROW_SIZE, size_t OTHER_COL_SIZE, MatrixMajorOrder OTHER_ORDER, size_t OTHER_ALIGN>
	requires (OTHER_ROW_SIZE > 0 && OTHER_ROW_SIZE <= ROW_SIZE && OTHER_COL_SIZE > 0 && OTHER_COL_SIZE <= COL_SIZE)
	void SetTopLeft(const MatrixType<T, OTHER_ROW_SIZE, OTHER_COL_SIZE, OTHER_ORDER, OTHER_ALIGN>& other)
	{
		for (size_t r = 0; r < OTHER_ROW_SIZE; r++)
		{
			for (size_t c = 0; c < OTHER_COL_SIZE; c++)
			{
				SetUnsafe(r, c, other.GetUnsafe(r, c));
			}
		}
	}
#pragma endregion

#pragma region MathOperations
	// --------------------------------------------------------------------------------------------------------
	//										MATRIX MATH OPERATIONS
	// --------------------------------------------------------------------------------------------------------

	/// <summary>
	/// Transpose moves the elements so an element at pos [R, C] -> [C, R]
	/// </summary>
	/// <returns></returns>
	MatrixType<T, COL_SIZE, ROW_SIZE, ORDER, ORDER_ALIGN_BYTES> Transpose() const
	{
		MatrixType<T, COL_SIZE, ROW_SIZE, ORDER, ORDER_ALIGN_BYTES> result = {};
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			for (size_t c = 0; c < COL_SIZE; c++)
			{
				result.SetUnsafe(c, r, GetUnsafe(r, c));
			}
		}
		return result;
	}

	bool Inverse(MatrixType& outResult) requires (ROW_SIZE == 2 && COL_SIZE == 2)
	{
		const float determinant = CalculateDeterminant();
		if (fabs(determinant) < DETERMINANT_EPSILON)
			return false;

		const float inverseDeterminant = 1 / determinant;
		std::array<std::array<T, COL_SIZE>, ROW_SIZE> newArr = 
		{{
			{GetUnsafe(1,1) * inverseDeterminant, -GetUnsafe(0,1) * inverseDeterminant},
			{-GetUnsafe(1,0) * inverseDeterminant, GetUnsafe(0,0) * inverseDeterminant}
		}};
		outResult.SetTopLeft(newArr);
		return true;
	}
	bool Inverse(MatrixType& outResult) requires (ROW_SIZE == 3 && COL_SIZE == 3)
	{
		const float determinant = CalculateDeterminant();

		if (fabs(determinant) < DETERMINANT_EPSILON)
			return false;

		const float inverseDeterminant = 1 / determinant;
		//While we could use create matrix out of cofactors and transpose, we get adjugate (transpose of cofactors)
		//directy so we skip creating an intermediate matrix
		std::array<std::array<T, COL_SIZE>, ROW_SIZE> newArr =
		{{
			{
				(GetUnsafe(1,1) * GetUnsafe(2,2) - GetUnsafe(1,2) * GetUnsafe(2,1))* inverseDeterminant,
				-(GetUnsafe(1,0) * GetUnsafe(2,2) - GetUnsafe(1,2) * GetUnsafe(2,0)) * inverseDeterminant,
				(GetUnsafe(1,0) * GetUnsafe(2,1) - GetUnsafe(1,1) * GetUnsafe(2,0))* inverseDeterminant
			},
			{
				-(GetUnsafe(0,1) * GetUnsafe(2,2) - GetUnsafe(0,2) * GetUnsafe(2,1)) * inverseDeterminant,
				(GetUnsafe(0,0) * GetUnsafe(2,2) - GetUnsafe(0,2) * GetUnsafe(2,0)) * inverseDeterminant,
				-(GetUnsafe(0,0) * GetUnsafe(2,1) - GetUnsafe(0,1) * GetUnsafe(2,0)) * inverseDeterminant
			},
			{
				(GetUnsafe(0,1) * GetUnsafe(1,2) - GetUnsafe(0,2) * GetUnsafe(1,1)) * inverseDeterminant,
				-(GetUnsafe(0,0) * GetUnsafe(1,2) - GetUnsafe(0,2) * GetUnsafe(1,0)) * inverseDeterminant,
				(GetUnsafe(0,0) * GetUnsafe(1,1) - GetUnsafe(0,1) * GetUnsafe(1,0)) * inverseDeterminant
			}
		}};
		outResult.SetTopLeft(newArr);

		return true;
	}

	bool Inverse(MatrixType& outResult) requires (ROW_SIZE == 4 && COL_SIZE == 4)
	{
		const float m00 = GetUnsafe(0, 0), m01 = GetUnsafe(0, 1), m02 = GetUnsafe(0, 2), m03 = GetUnsafe(0, 3);
		const float m10 = GetUnsafe(1, 0), m11 = GetUnsafe(1, 1), m12 = GetUnsafe(1, 2), m13 = GetUnsafe(1, 3);
		const float m20 = GetUnsafe(2, 0), m21 = GetUnsafe(2, 1), m22 = GetUnsafe(2, 2), m23 = GetUnsafe(2, 3);
		const float m30 = GetUnsafe(3, 0), m31 = GetUnsafe(3, 1), m32 = GetUnsafe(3, 2), m33 = GetUnsafe(3, 3);

		const float c00 = m11 * (m22 * m33 - m23 * m32) - m12 * (m21 * m33 - m23 * m31) + m13 * (m21 * m32 - m22 * m31);
		const float c01 = -m10 * (m22 * m33 - m23 * m32) + m12 * (m20 * m33 - m23 * m30) - m13 * (m20 * m32 - m22 * m30);
		const float c02 = m10 * (m21 * m33 - m23 * m31) - m11 * (m20 * m33 - m23 * m30) + m13 * (m20 * m31 - m21 * m30);
		const float c03 = -m10 * (m21 * m32 - m22 * m31) + m11 * (m20 * m32 - m22 * m30) - m12 * (m20 * m31 - m21 * m30);

		const float determinant = m00 * c00 + m01 * c01 + m02 * c02 + m03 * c03;
		if (fabs(determinant) < DETERMINANT_EPSILON)
			return false;

		const float inverseDeterminant = 1.0f / determinant;

		std::array<std::array<T, COL_SIZE>, ROW_SIZE> newArr =
		{ {
			{ c00 * inverseDeterminant,
			  -(m01 * (m22 * m33 - m23 * m32) - m02 * (m21 * m33 - m23 * m31) + m03 * (m21 * m32 - m22 * m31)) * inverseDeterminant,
			   (m01 * (m12 * m33 - m13 * m32) - m02 * (m11 * m33 - m13 * m31) + m03 * (m11 * m32 - m12 * m31)) * inverseDeterminant,
			  -(m01 * (m12 * m23 - m13 * m22) - m02 * (m11 * m23 - m13 * m21) + m03 * (m11 * m22 - m12 * m21)) * inverseDeterminant},

			{ c01 * inverseDeterminant,
			   (m00 * (m22 * m33 - m23 * m32) - m02 * (m20 * m33 - m23 * m30) + m03 * (m20 * m32 - m22 * m30)) * inverseDeterminant,
			  -(m00 * (m12 * m33 - m13 * m32) - m02 * (m10 * m33 - m13 * m30) + m03 * (m10 * m32 - m12 * m30)) * inverseDeterminant,
			   (m00 * (m12 * m23 - m13 * m22) - m02 * (m10 * m23 - m13 * m20) + m03 * (m10 * m22 - m12 * m20)) * inverseDeterminant},

			{ c02 * inverseDeterminant,
			  -(m00 * (m21 * m33 - m23 * m31) - m01 * (m20 * m33 - m23 * m30) + m03 * (m20 * m31 - m21 * m30)) * inverseDeterminant,
			   (m00 * (m11 * m33 - m13 * m31) - m01 * (m10 * m33 - m13 * m30) + m03 * (m10 * m31 - m11 * m30)) * inverseDeterminant,
			  -(m00 * (m11 * m23 - m13 * m21) - m01 * (m10 * m23 - m13 * m20) + m03 * (m10 * m21 - m11 * m20)) * inverseDeterminant},

			{ c03 * inverseDeterminant,
			   (m00 * (m21 * m32 - m22 * m31) - m01 * (m20 * m32 - m22 * m30) + m02 * (m20 * m31 - m21 * m30)) * inverseDeterminant,
			  -(m00 * (m11 * m32 - m12 * m31) - m01 * (m10 * m32 - m12 * m30) + m02 * (m10 * m31 - m11 * m30)) * inverseDeterminant,
			   (m00 * (m11 * m22 - m12 * m21) - m01 * (m10 * m22 - m12 * m20) + m02 * (m10 * m21 - m11 * m20)) * inverseDeterminant}
		} };

		outResult.SetTopLeft(newArr);
		return true;
	}

	bool Inverse() requires (ROW_SIZE >= 2 && ROW_SIZE <= 4 && COL_SIZE >= 2 && COL_SIZE <= 4 && ROW_SIZE == COL_SIZE)
	{
		return Inverse(*this);
	}

	MatrixType InverseUnsafe() 
		requires (ROW_SIZE >= 2 && ROW_SIZE <= 4 && COL_SIZE >= 2 && COL_SIZE <= 4 && ROW_SIZE == COL_SIZE)
	{
		MatrixType result = {};
		const bool inverseSuccess = Inverse(result);
		ENGINE_ASSERT(inverseSuccess, "InverseUnsafe failed due to singular matrix (determinant 0)");
		return result;
	}

	template<MatrixMajorOrder OTHER_ORDER, size_t OTHER_ALIGN>
	MatrixType operator+(const MatrixType<T, ROW_SIZE, COL_SIZE, OTHER_ORDER, OTHER_ALIGN>& other) const
	{
		MatrixType result = {};
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			for (size_t c = 0; c < COL_SIZE; c++)
			{
				result.SetUnsafe(r, c, GetUnsafe(r, c) + other.GetUnsafe(r, c));
			}
		}
		return result;
	}
	template<MatrixMajorOrder OTHER_ORDER, size_t OTHER_ALIGN>
	MatrixType& operator+=(const MatrixType<T, ROW_SIZE, COL_SIZE, OTHER_ORDER, OTHER_ALIGN>& other)
	{
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			for (size_t c = 0; c < COL_SIZE; c++)
			{
				SetUnsafe(r, c, GetUnsafe(r, c) + other.GetUnsafe(r, c));
			}
		}
		return *this;
	}

	MatrixType operator-() const
	{
		MatrixType result = {};
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			for (size_t c = 0; c < COL_SIZE; c++)
			{
				result.SetUnsafe(r, c, -GetUnsafe(r, c));
			}
		}
		return result;
	}

	template<MatrixMajorOrder OTHER_ORDER, size_t OTHER_ALIGN>
	MatrixType operator-(const MatrixType<T,ROW_SIZE, COL_SIZE, OTHER_ORDER, OTHER_ALIGN>& other) const
	{
		MatrixType result = {};
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			for (size_t c = 0; c < COL_SIZE; c++)
			{
				result.SetUnsafe(r, c, GetUnsafe(r, c) - other.GetUnsafe(r, c));
			}
		}
		return result;
	}
	template<MatrixMajorOrder OTHER_ORDER, size_t OTHER_ALIGN>
	MatrixType& operator-=(const MatrixType<T,ROW_SIZE, COL_SIZE, OTHER_ORDER, OTHER_ALIGN>& other)
	{
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			for (size_t c = 0; c < COL_SIZE; c++)
			{
				SetUnsafe(r, c, GetUnsafe(r, c) - other.GetUnsafe(r, c));
			}
		}
		return *this;
	}

	MatrixType operator*(const T scalar) const
	{
		MatrixType result = {};
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			for (size_t c = 0; c < COL_SIZE; c++)
			{
				result.SetUnsafe(r, c, GetUnsafe(r, c) * scalar);
			}
		}
		return result;
	}
	MatrixType& operator*=(const T scalar)
	{
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			for (size_t c = 0; c < COL_SIZE; c++)
			{
				SetUnsafe(r, c, GetUnsafe(r, c) * scalar);
			}
		}
		return *this;
	}

	template<size_t OTHER_ROW_SIZE, size_t OTHER_COL_SIZE, MatrixMajorOrder OTHER_ORDER, size_t OTHER_ALIGN>
	requires (IsPositiveSize<OTHER_ROW_SIZE, OTHER_COL_SIZE> && COL_SIZE == OTHER_ROW_SIZE)
	MatrixType<T,ROW_SIZE, OTHER_COL_SIZE, ORDER, ORDER_ALIGN_BYTES> 
		operator*(const MatrixType<T,OTHER_ROW_SIZE, OTHER_COL_SIZE, OTHER_ORDER, OTHER_ALIGN>& other) const
	{
		MatrixType<T,ROW_SIZE, OTHER_COL_SIZE, ORDER, ORDER_ALIGN_BYTES> result = {};
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			for (size_t otherC = 0; otherC < OTHER_COL_SIZE; otherC++)
			{
				for (size_t c = 0; c < COL_SIZE; c++)
				{
					result.SetUnsafe(r, otherC, result.GetUnsafe(r, otherC) + GetUnsafe(r, c) * other.GetUnsafe(c, otherC));
				}
			}
		}
		return result;
	}

	template<size_t VEC_SIZE>
	requires (VEC_SIZE >0 && VEC_SIZE == COL_SIZE && std::is_default_constructible_v<Vec<T, ROW_SIZE>>)
	Vec<T, ROW_SIZE> operator*(const Vec<T, VEC_SIZE>& vec) const
	{
		Vec<T, ROW_SIZE> result = {};
		for (size_t c = 0; c < COL_SIZE; c++)
		{
			for (size_t r = 0; r < ROW_SIZE; r++)
			{
				result[r] += GetUnsafe(r, c) * vec[c];
			}
		}
		return result;
	}
#pragma endregion

	std::string ToString(const bool newLineOnRow=true) const
	{
		std::string result = "";
		for (size_t r = 0; r < ROW_SIZE; r++)
		{
			if (newLineOnRow) result += '\n';
			result += '[' + std::to_string(GetUnsafe(r, 0));

			for (size_t c = 1; c < COL_SIZE; c++)
			{
				result += ',' + std::to_string(GetUnsafe(r, c));
			}
			result += ']';
		}
		
		if (newLineOnRow) result += '\n';
		return result;
	}
};

using Mat4 = MatrixType<float, 4, 4, MatrixMajorOrder::Column>;
using Mat3 = MatrixType<float, 3, 3, MatrixMajorOrder::Column>;