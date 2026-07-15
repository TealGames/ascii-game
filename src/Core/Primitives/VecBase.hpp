#pragma once
#include <type_traits>
#include <format>
#include "Utils/TemplateConcepts.hpp"
#include "Utils/Math.hpp"
#include "Utils/ToStringFunctions.hpp"

namespace Engine
{
	template<typename T, size_t N, size_t ALIGN_BYTES = alignof(T)>
		requires (std::is_arithmetic_v<T> && N > 0)
	class alignas(ALIGN_BYTES) Vec
	{
	private:
	public:
		T m_Components[N] = {};

	private:
	public:
		constexpr Vec(const Vec&) = default;
		Vec(Vec&&) noexcept = default;

		constexpr T& operator[](const size_t index)
		{
			return m_Components[index];
		}
		constexpr const T& operator[](const size_t index) const
		{
			return m_Components[index];
		}

		std::string ToString() const
		{
			std::string result = '(' + std::to_string(m_Components[0]);
			for (size_t i = 1; i < N; i++)
			{
				result += ',' + std::to_string(m_Components[i]);
			}
			result += ')';
			return result;
		}
	};

	//-----------------------------------------------------------------------------------------------------------------------------------
	//															VECTOR2 DEFINITION
	//-----------------------------------------------------------------------------------------------------------------------------------
	template<typename T, size_t ALIGN_BYTES>
		requires (std::is_arithmetic_v<T> && ALIGN_BYTES >= alignof(T))
	class alignas(ALIGN_BYTES) Vec<T, 2, ALIGN_BYTES>
	{
	private:
	public:
		T m_X, m_Y;

	public:
		constexpr Vec() : Vec(0, 0) {}
		constexpr Vec(const T xy) : Vec(xy, xy) {}
		constexpr Vec(const T x, const T y)
			: m_X(x), m_Y(y) {}

		constexpr Vec(const std::array<T, 2>& arr) : Vec(arr[0], arr[1]) {}

		template<typename TOther, size_t OTHER_ALIGN>
			requires (!std::is_same_v<T, TOther> && std::is_convertible_v<TOther, T>)
					 || (OTHER_ALIGN != ALIGN_BYTES)
		constexpr Vec(const Vec<TOther, 2, OTHER_ALIGN>& other)
			: Vec(other.m_X, other.m_Y) {}

		constexpr Vec(const Vec&) = default;
		Vec(Vec&&) noexcept = default;

		constexpr Vec GetX() const { return Vec(m_X, 0); }
		constexpr Vec GetY() const { return Vec(0, m_Y); }

		constexpr Vec<int, 2> AsInt() const requires (std::is_floating_point_v<T>)
		{
			return Vec<int, 2>(m_X, m_Y);
		}
		constexpr Vec<float, 2> AsFloat() const requires (std::is_integral_v<T>)
		{
			return Vec<float, 2>(m_X, m_Y);
		}

		static constexpr Vec One() { return Vec{ 1, 1 }; }
		static constexpr Vec Zero() { return Vec{ 0, 0 }; }

		const T* GetMemPointer() const { return &m_X; }
		T* GetMemPointerMutable() { return &m_X; }

		/// <summary>
		/// Computes the magnitude squared. 
		/// Better for performance because avoids expensive sqrt.
		/// Use when comparing magnitudes and don't need real value.
		/// Same as dot product of itself
		/// </summary>
		/// <returns></returns>
		constexpr T GetMagnitudeSquared() const { return m_X * m_X + m_Y * m_Y; }
		float GetMagnitude() const { return std::sqrt(GetMagnitudeSquared()); }

		Vec<float, 2> GetNormalized() const
		{
			const float magnitude = GetMagnitude();
			if (::Math::ApproximateEqualsF(magnitude, 0))
				return {};

			float inverseMag = 1 / magnitude;
			return *this * inverseMag;
		}

		const Vec& Normalize()
		{
			const float magnitude = GetMagnitude();
			if (::Math::ApproximateEqualsF(magnitude, 0))
				return;
			
			//TODO: since vec2 can be optimized as much it might be better to just do the multiplies in place
			//like m_X *= inverseMag, etc rather than invoking *= operator
			float inverseMag = 1 / magnitude;
			*this *= inverseMag;
			return *this;
		}

		std::string ToString(const std::uint8_t& decimalPlaces = 5) const
		{
			std::string str;
			std::string xRounded = ::Utils::ToString(::Math::Roundf(m_X, decimalPlaces), decimalPlaces);
			std::string yRounded = ::Utils::ToString(::Math::Roundf(m_Y, decimalPlaces), decimalPlaces);
			return std::format("({},{})", xRounded, yRounded);
		}

		constexpr T& operator[](const size_t index)
		{
			ENGINE_ASSERT(index <= 1, "Invalid vec index:{}", index);
			return (&m_X)[index];
		}
		constexpr const T& operator[](const size_t index) const
		{
			ENGINE_ASSERT(index <= 1, "Invalid vec index:{}", index);
			return (&m_X)[index];
		}

		template<size_t INDEX> 
		requires (INDEX < 2)
		constexpr T& Get() { return (&m_X)[INDEX]; }

		template<size_t INDEX>
			requires (INDEX < 2)
		constexpr const T& Get() const { return (&m_X)[INDEX]; }

		template<typename TRhs, size_t RHS_ALIGN>
		constexpr Vec& operator+=(const Vec<TRhs, 2, RHS_ALIGN>& rhs)
		{
			m_X += rhs.m_X;
			m_Y += rhs.m_Y;
			return *this;
		}
		constexpr Vec& operator-()
		{
			m_X = -m_X;
			m_Y = -m_Y;
			return *this;
		}
		template<typename TRhs, size_t RHS_ALIGN>
		constexpr Vec& operator-=(const Vec<TRhs, 2, RHS_ALIGN>& rhs)
		{
			m_X -= rhs.m_X;
			m_Y -= rhs.m_Y;
			return *this;
		}

		template<typename TRhs, size_t RHS_ALIGN>
		constexpr Vec& operator*=(const Vec<TRhs, 2, RHS_ALIGN>& rhs)
		{
			m_X *= rhs.m_X;
			m_Y *= rhs.m_Y;
			return *this;
		}
		template<typename TScalar>
			requires (std::is_arithmetic_v<TScalar>)
		constexpr Vec& operator*=(TScalar rhs)
		{
			m_X *= rhs;
			m_Y *= rhs;
			return *this;
		}

		template<typename TRhs, size_t RHS_ALIGN>
		constexpr Vec& operator/=(const Vec<TRhs, 2, RHS_ALIGN>& rhs)
		{
			if constexpr (std::is_floating_point_v<TRhs>)
			{
				ENGINE_ASSERT(!::Math::ApproximateEqualsF(rhs.m_X, 0) && !::Math::ApproximateEqualsF(rhs.m_Y, 0),
					"Tried to divide a vector:{} by a 0-value vector component: {}", ToString(), rhs.ToString());
			}
			else
			{
				ENGINE_ASSERT(rhs.m_X != 0 && rhs.m_Y != 0,
					"Tried to divide a vector:{} by a 0-value vector component: {}", ToString(), rhs.ToString());
			}

			m_X /= rhs.m_X;
			m_Y /= rhs.m_Y;
			return *this;
		}
		template<typename TScalar>
			requires (std::is_arithmetic_v<TScalar>)
		constexpr  Vec& operator/=(TScalar rhs)
		{
			if constexpr (std::is_floating_point_v<TScalar>)
			{
				ENGINE_ASSERT(!::Math::ApproximateEqualsF(rhs, 0),
					"Tried to divide a vector:{} by a 0-value scalar: {}", ToString(), rhs);
			}
			else
			{
				ENGINE_ASSERT(rhs != 0, "Tried to divide a vector:{} by a 0-value scalar: {}", ToString(), rhs);
			}

			m_X /= rhs;
			m_Y /= rhs;
			return *this;
		}

		constexpr bool AnyAxisGreaterThan(const Vec& other) const
		{
			return m_X > other.m_X || m_Y > other.m_Y;
		}
		constexpr bool AnyAxisLessThan(const Vec& other) const
		{
			return m_X < other.m_X || m_Y < other.m_Y;
		}

		Vec& operator=(const Vec& other) = default;
		Vec& operator=(Vec&& other) noexcept = default;
	};

	//-----------------------------------------------------------------------------------------------------------------------------------
	//															VECTOR3 DEFINITION
	//-----------------------------------------------------------------------------------------------------------------------------------
	template<typename T, size_t ALIGN_BYTES>
		requires (std::is_arithmetic_v<T> && ALIGN_BYTES >= alignof(T))
	class alignas(ALIGN_BYTES) Vec<T, 3, ALIGN_BYTES>
	{
	public:
		T m_X, m_Y, m_Z;

	public:
		constexpr Vec() : Vec(0, 0, 0) {}
		constexpr Vec(const T xyz) : m_X(xyz), m_Y(xyz), m_Z(xyz) {}
		constexpr Vec(const T x, const T y, const T z) : m_X(x), m_Y(y), m_Z(z) {}

		constexpr Vec(const std::array<T, 3>& arr) : m_X(arr[0]), m_Y(arr[1]), m_Z(arr[2]) {}

		constexpr Vec(const Vec<T, 2>& xy, const T zComp)
			: Vec(xy.m_X, xy.m_Y, zComp) {}

		constexpr Vec(const T xComp, const Vec<T, 2>& yz)
			: Vec(xComp, yz.m_X, yz.m_Y) {}

		constexpr Vec(const Vec&) = default;
		Vec(Vec&&) noexcept = default;

		template<typename TOther, size_t OTHER_ALIGN>
			requires (!std::is_same_v<T, TOther> && std::is_convertible_v<TOther, T>) 
					 || (OTHER_ALIGN != ALIGN_BYTES)
		constexpr Vec(const Vec<TOther, 3, OTHER_ALIGN>& other)
			: Vec(other.m_X, other.m_Y, other.m_Z) {}

		template<size_t OTHER_ALIGN>
			requires (OTHER_ALIGN != ALIGN_BYTES && std::is_default_constructible_v<T>)
		Vec(Vec<T, 3, OTHER_ALIGN>&& other) noexcept
			: m_X(std::move(other.m_X)), m_Y(std::move(other.m_Y)), m_Z(std::move(other.m_Z)) {}

		static inline constexpr Vec One() { return Vec{ 1, 1, 1 }; }
		static inline constexpr Vec Zero() { return Vec{ 0, 0, 0 }; }

		/// <summary>
		/// Returns a vector where each component is the MIN
		/// possible value for the specified T type
		/// </summary>
		/// <returns></returns>
		static inline constexpr Vec Min()
		{
			T tMin = std::numeric_limits<T>::min();
			return { tMin, tMin, tMin };
		}
		/// <summary>
		/// Returns a vector where each component is the MAX
		/// possible value for the specified T type
		/// </summary>
		/// <returns></returns>
		static inline constexpr Vec Max()
		{
			T tMax = std::numeric_limits<T>::max();
			return { tMax, tMax, tMax };
		}

		constexpr Vec GetX() const { return Vec(m_X, 0, 0); }
		constexpr Vec GetY() const { return Vec(0, m_Y, 0); }
		constexpr Vec GetZ() const { return Vec(0, 0, m_Z); }

		constexpr Vec<T, 2> GetXY() const { return Vec<T, 2>(m_X, m_Y); }
		constexpr Vec<T, 2> GetYZ() const { return Vec<T, 2>(m_Y, m_Z); }
		constexpr Vec<T, 2> GetXZ() const { return Vec<T, 2>(m_X, m_Z); }

		constexpr Vec<int, 3> AsInt() const requires (std::is_floating_point_v<T>)
		{
			return Vec<int, 3>(m_X, m_Y, m_Z);
		}
		constexpr Vec<float, 3> AsFloat() const requires (std::is_integral_v<T>)
		{
			return Vec<float, 3>(m_X, m_Y, m_Z);
		}

		void SetXY(const Vec<T, 2> vec)
		{
			m_X = vec.m_X;
			m_Y = vec.m_Y;
		}
		void SetYZ(const Vec<T, 2> vec)
		{
			m_Y = vec.m_X;
			m_Z = vec.m_Y;
		}

		/// <summary>
		/// Returns a pointer to the first value in vector, 
		/// allowing user to treat vector components as array due to 
		/// them being packed right after one another in memory.
		/// </summary>
		/// <returns></returns>
		const T* GetMemPointer() const { return &m_X; }
		T* GetMemPointerMutable() { return &m_X; }

		bool IsUniform() const
		{
			if constexpr (std::is_floating_point_v<T>)
				return ::Math::ApproximateEqualsF(m_X, m_Y) && ::Math::ApproximateEqualsF(m_Y, m_Z);
			return m_X == m_Y && m_Y == m_Z;
		}

		float GetMaxComponentValue() const
		{
			return std::max(std::max(m_X, m_Y), m_Z);
		}
		std::uint8_t GetMaxComponentIndex() const
		{
			if (m_X >= m_Y && m_X >= m_Z)
				return 0;
			if (m_Y >= m_X && m_Y >= m_Z)
				return 1;
			return 2;
		}
		float GetMinComponentValue() const
		{
			return std::min(std::min(m_X, m_Y), m_Z);
		}

		/// <summary>
		/// Computes the magnitude squared. 
		/// Better for performance because avoids expensive sqrt.
		/// Use when comparing magnitudes and don't need real value.
		/// Same as dot product of itself
		/// </summary>
		/// <returns></returns>
		constexpr T GetMagnitudeSquared() const { return m_X * m_X + m_Y * m_Y + m_Z * m_Z; }
		float GetMagnitude() const { return std::sqrt(GetMagnitudeSquared()); }

		Vec<float, 3> GetNormalized() const
		{
			const float magnitude = GetMagnitude();
			if (::Math::ApproximateEqualsF(magnitude, 0))
				return {};

			float inverseMag = 1 / magnitude;
			return *this * inverseMag;
		}

		const Vec& Normalize()
		{
			const float magnitude = GetMagnitude();
			if (::Math::ApproximateEqualsF(magnitude, 0))
				return *this;

			//TODO: since vec2 can be optimized as much it might be better to just do the multiplies in place
			//like m_X *= inverseMag, etc rather than invoking *= operator
			float inverseMag = 1 / magnitude;
			*this *= inverseMag;
			return *this;
		}

		std::string ToString(const std::uint8_t decimalPlaces = 5) const
		{
			std::string str;
			std::string xRounded = ::Utils::ToString(::Math::Roundf(m_X, decimalPlaces), decimalPlaces);
			std::string yRounded = ::Utils::ToString(::Math::Roundf(m_Y, decimalPlaces), decimalPlaces);
			std::string zRounded = ::Utils::ToString(::Math::Roundf(m_Z, decimalPlaces), decimalPlaces);
			return std::format("({},{},{})", xRounded, yRounded, zRounded);
		}

		T& operator[](const size_t index)
		{
			ENGINE_ASSERT(index <= 2, "Invalid vec index:{}", index);
			return (&m_X)[index];
		}
		const T& operator[](const size_t index) const
		{
			ENGINE_ASSERT(index <= 2, "Invalid vec index:{}", index);
			return (&m_X)[index];
		}

		template<size_t INDEX>
			requires (INDEX < 3)
		constexpr T& Get() { return (&m_X)[INDEX]; }

		template<size_t INDEX>
			requires (INDEX < 3)
		constexpr const T& Get() const { return (&m_X)[INDEX]; }

		template<typename TRhs, size_t RHS_ALIGN>
		constexpr Vec& operator+=(const Vec<TRhs, 3, RHS_ALIGN>& rhs)
		{
			m_X += rhs.m_X;
			m_Y += rhs.m_Y;
			m_Z += rhs.m_Z;
			return *this;
		}
		template<typename TRhs, size_t RHS_ALIGN>
		constexpr Vec& operator-=(const Vec<TRhs, 3, RHS_ALIGN>& rhs)
		{
			m_X -= rhs.m_X;
			m_Y -= rhs.m_Y;
			m_Z -= rhs.m_Z;
			return *this;
		}

		template<typename TRhs, size_t RHS_ALIGN>
		constexpr Vec& operator*=(const Vec<TRhs, 3, RHS_ALIGN>& rhs)
		{
			m_X *= rhs.m_X;
			m_Y *= rhs.m_Y;
			m_Z *= rhs.m_Z;
			return *this;
		}
		template<typename TScalar>
			requires (std::is_arithmetic_v<TScalar>)
		constexpr Vec& operator*=(TScalar rhs)
		{
			m_X *= rhs;
			m_Y *= rhs;
			m_Z *= rhs;
			return *this;
		}

		template<typename TRhs, size_t RHS_ALIGN>
		constexpr Vec& operator/=(const Vec<TRhs, 3, RHS_ALIGN>& rhs)
		{
			if constexpr (std::is_floating_point_v<TRhs>)
			{
				ENGINE_ASSERT(!::Math::ApproximateEqualsF(rhs.m_X, 0) && !::Math::ApproximateEqualsF(rhs.m_Y, 0) &&
					!::Math::ApproximateEqualsF(rhs.m_Z, 0),
					"Tried to divide a vector:{} by a 0-value vector component: {}", ToString(), rhs.ToString());
			}
			else
			{
				ENGINE_ASSERT(rhs.m_X != 0 && rhs.m_Y != 0 && rhs.m_Z != 0,
					"Tried to divide a vector:{} by a 0-value vector component: {}", ToString(), rhs.ToString());
			}

			m_X /= rhs.m_X;
			m_Y /= rhs.m_Y;
			m_Z /= rhs.m_Z;
			return *this;
		}
		template<typename TScalar>
			requires (std::is_arithmetic_v<TScalar>)
		constexpr  Vec& operator/=(TScalar rhs)
		{
			if constexpr (std::is_floating_point_v<TScalar>)
			{
				ENGINE_ASSERT(!::Math::ApproximateEqualsF(rhs, 0),
					"Tried to divide a vector:{} by a 0-value scalar: {}", ToString(), rhs);
			}
			else
			{
				ENGINE_ASSERT(rhs != 0, "Tried to divide a vector:{} by a 0-value scalar: {}", ToString(), rhs);
			}

			m_X /= rhs;
			m_Y /= rhs;
			m_Z /= rhs;
			return *this;
		}

		constexpr bool AnyAxisGreaterThan(const Vec& other) const
		{
			return m_X > other.m_X || m_Y > other.m_Y || m_Z > other.m_Z;
		}
		constexpr bool AnyAxisLessThan(const Vec& other) const
		{
			return m_X < other.m_X || m_Y < other.m_Y || m_Z < other.m_Z;
		}

		Vec& operator=(const Vec& other) = default;
		Vec& operator=(Vec&& other) noexcept = default;
	};

	//-----------------------------------------------------------------------------------------------------------------------------------
	//															VECTOR4 DEFINITION
	//-----------------------------------------------------------------------------------------------------------------------------------
	template<typename T, size_t ALIGN_BYTES>
		requires (std::is_arithmetic_v<T> && ALIGN_BYTES >= alignof(T))
	class alignas(ALIGN_BYTES) Vec<T, 4, ALIGN_BYTES>
	{
	private:
	public:
		T m_X, m_Y, m_Z, m_W;

	public:
		constexpr Vec() : Vec(0, 0, 0, 0) {}
		constexpr Vec(const T xyzw) : Vec(xyzw, xyzw, xyzw, xyzw) {}
		constexpr Vec(const T xComp, const T yComp, const T zComp, const T wComp)
			: m_X(xComp), m_Y(yComp), m_Z(zComp), m_W(wComp) {}

		constexpr Vec(const std::array<T, 4>& arr)
			: Vec(arr[0], arr[1], arr[2], arr[3]) {}

		constexpr Vec(const Vec<T, 2>& xy, const T zComp, const T wComp)
			: Vec(xy.m_X, xy.m_Y, zComp, wComp) {}

		constexpr Vec(const T xComp, const Vec<T, 2>& yz, const T wComp)
			: Vec(xComp, yz.m_X, yz.m_Y, wComp) {}

		constexpr Vec(const T xComp, const T yComp, const Vec<T, 2>& zw)
			: Vec(xComp, yComp, zw.m_X, zw.m_Y) {}

		constexpr Vec(const Vec<T, 3>& xyz, const T wComp)
			: Vec(xyz.m_X, xyz.m_Y, xyz.m_Z, wComp) {}

		constexpr Vec(const T xComp, const Vec<T, 3>& yzw)
			: Vec(xComp, yzw.m_X, yzw.m_Y, yzw.m_W) {}

		template<typename TOther, size_t OTHER_ALIGN>
		requires (!std::is_same_v<T, TOther> && std::is_convertible_v<TOther, T>)
				 || (OTHER_ALIGN != ALIGN_BYTES)
		constexpr Vec(const Vec<TOther, 4, OTHER_ALIGN>& other)
			: Vec(other.m_X, other.m_Y, other.m_Z, other.m_W) {}

		constexpr Vec(const Vec&) = default;
		Vec(Vec&&) noexcept = default;

		static inline constexpr Vec Zero() { return { 0, 0, 0, 0 }; }
		static inline constexpr Vec One() { return { 1, 1, 1, 1 }; }

		constexpr Vec GetX() const { return Vec(m_X, 0, 0, 0); }
		constexpr Vec GetY() const { return Vec(0, m_Y, 0, 0); }
		constexpr Vec GetZ() const { return Vec(0, 0, m_Z, 0); }
		constexpr Vec GetW() const { return Vec(0, 0, 0, m_W); }

		constexpr Vec<T, 2> GetXY() const { return Vec<T, 2>(m_X, m_Y); }
		constexpr Vec<T, 2> GetYZ() const { return Vec<T, 2>(m_Y, m_Z); }
		constexpr Vec<T, 2> GetXZ() const { return Vec<T, 2>(m_X, m_Z); }

		constexpr Vec<T, 3> GetXYZ() const { return Vec<T, 3>(m_X, m_Y, m_Z); }
		constexpr Vec<T, 3> GetYZW() const { return Vec<T, 3>(m_Y, m_Z, m_W); }

		/// <summary>
		/// Returns a pointer to the first value in vector, 
		/// allowing user to treat vector components as array due to 
		/// them being packed right after one another in memory.
		/// </summary>
		/// <returns></returns>
		const T* GetMemPointer() const { return &m_X; }

		/// <summary>
		/// Computes the magnitude squared. 
		/// Better for performance because avoids expensive sqrt.
		/// Use when comparing magnitudes and don't need real value.
		/// Same as dot product of itself
		/// </summary>
		/// <returns></returns>
		constexpr float GetMagnitudeSquared() const { return m_X * m_X + m_Y * m_Y + m_Z * m_Z + m_W * m_W; }
		float GetMagnitude() const { return std::sqrt(GetMagnitudeSquared()); }

		Vec<float, 4> GetNormalized() const
		{
			const float magnitude = GetMagnitude();
			if (::Math::ApproximateEqualsF(magnitude, 0))
				return {};

			float inverseMag = 1 / magnitude;
			return *this * inverseMag;
		}

		const Vec& Normalize()
		{
			const float magnitude = GetMagnitude();
			if (::Math::ApproximateEqualsF(magnitude, 0))
				return;

			//TODO: since vec2 can be optimized as much it might be better to just do the multiplies in place
			//like m_X *= inverseMag, etc rather than invoking *= operator
			float inverseMag = 1 / magnitude;
			*this *= inverseMag;
			return *this;
		}

		std::string ToString(const std::uint8_t& decimalPlaces = 5) const
		{
			return std::format("({},{},{},{})",
				Utils::ToString(::Math::Roundf(m_X, decimalPlaces), decimalPlaces),
				Utils::ToString(::Math::Roundf(m_Y, decimalPlaces), decimalPlaces),
				Utils::ToString(::Math::Roundf(m_Z, decimalPlaces), decimalPlaces),
				Utils::ToString(::Math::Roundf(m_W, decimalPlaces), decimalPlaces));
		}

		T& operator[](const size_t index)
		{
			ENGINE_ASSERT(index <= 3, "Invalid vec index:{}", index);
			return (&m_X)[index];
		}
		const T& operator[](const size_t index) const
		{
			ENGINE_ASSERT(index <= 3, "Invalid vec index:{}", index);
			return (&m_X)[index];
		}

		template<size_t INDEX>
			requires (INDEX < 4)
		constexpr T& Get() { return (&m_X)[INDEX]; }

		template<size_t INDEX>
			requires (INDEX < 4)
		constexpr const T& Get() const { return (&m_X)[INDEX]; }

		template<typename TRhs, size_t RHS_ALIGN>
		constexpr Vec& operator+=(const Vec<TRhs, 4, RHS_ALIGN>& rhs)
		{
			m_X += rhs.m_X;
			m_Y += rhs.m_Y;
			m_Z += rhs.m_Z;
			m_W += rhs.m_W;
			return *this;
		}
		constexpr Vec& operator-()
		{
			m_X = -m_X;
			m_Y = -m_Y;
			m_Z = -m_Z;
			m_W = -m_W;
			return *this;
		}
		template<typename TRhs, size_t RHS_ALIGN>
		constexpr Vec& operator-=(const Vec<TRhs, 4, RHS_ALIGN>& rhs)
		{
			m_X -= rhs.m_X;
			m_Y -= rhs.m_Y;
			m_Z -= rhs.m_Z;
			m_W -= rhs.m_W;
			return *this;
		}

		template<typename TRhs, size_t RHS_ALIGN>
		constexpr Vec& operator*=(const Vec<TRhs, 4, RHS_ALIGN>& rhs)
		{
			m_X *= rhs.m_X;
			m_Y *= rhs.m_Y;
			m_Z *= rhs.m_Z;
			m_W *= rhs.m_W;
			return *this;
		}
		template<typename TScalar>
			requires (std::is_arithmetic_v<TScalar>)
		constexpr Vec& operator*=(TScalar rhs)
		{
			m_X *= rhs;
			m_Y *= rhs;
			m_Z *= rhs;
			m_W *= rhs;
			return *this;
		}

		template<typename TRhs, size_t RHS_ALIGN>
		constexpr Vec& operator/=(const Vec<TRhs, 4, RHS_ALIGN>& rhs)
		{
			if constexpr (std::is_floating_point_v<TRhs>)
			{
				ENGINE_ASSERT(!::Math::ApproximateEqualsF(rhs.m_X, 0) && !::Math::ApproximateEqualsF(rhs.m_Y, 0) &&
					!::Math::ApproximateEqualsF(rhs.m_Z, 0) && !::Math::ApproximateEqualsF(rhs.m_W, 0),
					"Tried to divide a vector:{} by a 0-value vector component: {}", ToString(), rhs.ToString());
			}
			else
			{
				ENGINE_ASSERT(rhs.m_X != 0 && rhs.m_Y !=0 && rhs.m_Z !=0 && rhs.m_W != 0,
					"Tried to divide a vector:{} by a 0-value vector component: {}", ToString(), rhs.ToString());
			}

			m_X /= rhs.m_X;
			m_Y /= rhs.m_Y;
			m_Z /= rhs.m_Z;
			m_W /= rhs.m_W;
			return *this;
		}
		template<typename TScalar>
			requires (std::is_arithmetic_v<TScalar>)
		constexpr  Vec& operator/=(TScalar rhs)
		{
			if constexpr (std::is_floating_point_v<TScalar>)
			{
				ENGINE_ASSERT(!::Math::ApproximateEqualsF(rhs, 0),
					"Tried to divide a vector:{} by a 0-value scalar: {}", ToString(), rhs);
			}
			else
			{
				ENGINE_ASSERT(rhs != 0, "Tried to divide a vector:{} by a 0-value scalar: {}", ToString(), rhs);
			}

			m_X /= rhs;
			m_Y /= rhs;
			m_Z /= rhs;
			m_W /= rhs;
			return *this;
		}

		Vec& operator=(const Vec& other) = default;
		Vec& operator=(Vec&& other) noexcept = default;
	};

	template<typename TLhs, size_t LHS_ALIGN, size_t N, typename TRhs, size_t RHS_ALIGN>
	constexpr auto operator+(const Vec<TLhs, N, LHS_ALIGN>& lhs, const Vec<TRhs, N, RHS_ALIGN>& rhs)
		-> Vec<std::common_type_t<TLhs, TRhs>, N>
	{
		using CommonType = std::common_type_t<TLhs, TRhs>;
		Vec<CommonType, N> result = lhs;
		result += rhs;
		return result;                                                                                                                                  
	}
	
	template<typename TLhs, size_t LHS_ALIGN, size_t N, typename TRhs, size_t RHS_ALIGN>
	constexpr auto operator-(const Vec<TLhs, N, LHS_ALIGN>& lhs, const Vec<TRhs, N, RHS_ALIGN>& rhs)
		-> Vec<std::common_type_t<TLhs, TRhs>, N>
	{
		using CommonType = std::common_type_t<TLhs, TRhs>;
		Vec<CommonType, N> result = lhs;
		result -= rhs;
		return result;
	}


	template<typename TLhs, size_t LHS_ALIGN, size_t N, typename TRhs, size_t RHS_ALIGN>
	constexpr auto operator*(const Vec<TLhs, N, LHS_ALIGN>& lhs, const Vec<TRhs, N, RHS_ALIGN>& rhs)
		-> Vec<std::common_type_t<TLhs, TRhs>, N>
	{
		using CommonType = std::common_type_t<TLhs, TRhs>;
		Vec<CommonType, N> result = lhs;
		result *= rhs;
		return result;
	}
	template<typename TScalar, typename TVec, size_t N, size_t ALIGN>
		requires (std::is_arithmetic_v<TScalar>)
	constexpr auto operator*(const Vec<TVec, N, ALIGN>& lhs, TScalar rhs)
		-> Vec<std::common_type_t<TScalar, TVec>, N>
	{
		using CommonType = std::common_type_t<TScalar, TVec>;
		Vec<CommonType, N> result = lhs;
		result *= rhs;
		return result;
	}
	template<typename TScalar, typename TVec, size_t N, size_t ALIGN>
		requires (std::is_arithmetic_v<TScalar>)
	constexpr auto operator*(TScalar lhs, const Vec<TVec, N, ALIGN>& rhs)
		-> Vec<std::common_type_t<TScalar, TVec>, N>
	{
		using CommonType = std::common_type_t<TScalar, TVec>;
		Vec<CommonType, N> result = rhs;
		result *= lhs;
		return result;
	}
	template<typename TLhs, size_t LHS_ALIGN, size_t N>
	constexpr Vec<TLhs, N, LHS_ALIGN> operator-(const Vec<TLhs, N, LHS_ALIGN>& lhs)
	{
		Vec<TLhs, N, LHS_ALIGN> result = lhs;
		result *= -1;
		return result;
	}

	template<typename TLhs, size_t LHS_ALIGN, size_t N, typename TRhs, size_t RHS_ALIGN>
	constexpr auto operator/(const Vec<TLhs, N, LHS_ALIGN>& lhs, const Vec<TRhs, N, RHS_ALIGN>& rhs)
		-> Vec<std::common_type_t<TLhs, TRhs>, N>
	{
		using CommonType = std::common_type_t<TLhs, TRhs>;
		Vec<CommonType, N> result = lhs;
		result /= rhs;
		return result;
	}

	template<typename TScalar, typename TVec, size_t N, size_t ALIGN>
		requires (std::is_arithmetic_v<TScalar>)
	constexpr auto operator/(const Vec<TVec, N, ALIGN>& lhs, TScalar rhs)
		-> Vec<std::common_type_t<TScalar, TVec>, N>
	{
		using CommonType = std::common_type_t<TScalar, TVec>;
		Vec<CommonType, N> result = lhs;
		result /= rhs;
		return result;
	}

	template<typename TLhs, size_t LHS_ALIGN, size_t N, typename TRhs, size_t RHS_ALIGN>
	bool operator==(const Vec<TLhs, N, LHS_ALIGN>& lhs, const Vec<TRhs, N, RHS_ALIGN>& rhs)
	{
		using CommonType = std::common_type_t<TLhs, TRhs>;
		for (size_t i = 0; i < N; i++)
		{
			if constexpr (std::is_floating_point_v<CommonType>)
			{
				if (!::Math::ApproximateEqualsF(lhs[i], rhs[i]))
					return false;
			}
			else
			{
				if (lhs[i] != rhs[i])
					return false;
			}
		}
		return true;
	}
	template<typename TLhs, size_t LHS_ALIGN, size_t N, typename TRhs, size_t RHS_ALIGN>
	bool operator!=(const Vec<TLhs, N, LHS_ALIGN>& lhs, const Vec<TRhs, N, RHS_ALIGN>& rhs)
	{
		return !(lhs == rhs);
	}
}
