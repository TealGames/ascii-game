#pragma once
#include <type_traits>
#include <format>
#include "Core/Primitives/Vector.hpp"

namespace Engine
{
	template<typename T, size_t N, size_t ALIGN_BYTES = alignof(T)>
		requires (std::is_arithmetic_v<T> && N > 0)
	struct alignas(ALIGN_BYTES) Col
	{
		T m_Channels[N];

		T& operator[](const size_t index)
		{
			if (index >= N)
				throw std::invalid_argument(std::format("Invalid col index:{}", index));
			return m_Channels[index];
		}
		const T& operator[](const size_t index) const
		{
			if (index >= N)
				throw std::invalid_argument(std::format("Invalid col index:{}", index));
			return m_Channels[index];
		}

		std::string ToString() const
		{
			std::string result = '(' + std::to_string(m_Channels[0]);
			for (size_t i = 1; i < m_Channels.size(); i++)
			{
				result += ',' + std::to_string(m_Channels[i]);
			}
			result += ')';
			return result;
		}
	};

	template<typename T, size_t ALIGN_BYTES>
		requires (std::is_arithmetic_v<T>)
	class alignas(ALIGN_BYTES) Col<T, 2, ALIGN_BYTES>
	{
	private:
	public:
		union
		{
			struct { T m_R, m_G; };
			T m_Channels[2];
		};

		static constexpr T MAX_CHANNEL_VALUE = std::numeric_limits<T>::max();

	public:
		constexpr Col()
			: m_R(), m_G() {}

		constexpr Col(const std::array<T, 2>& arr)
			: Col(arr[0], arr[1]) {}

		constexpr Col(const T r, const T g)
			: m_R(r), m_G(g) {}

		constexpr Col(const Vec<T, 2>& rg)
			: Col(rg.m_X, rg.m_Y) {}

		// ------------------------------------ FLOATING POINT ONLY CONSTRUCTORS --------------------------------
		constexpr Col(std::uint8_t r, std::uint8_t g)
			requires (std::is_floating_point_v<T>)
		: m_R(r / 255.0f), m_G(g / 255.0f) {}

		constexpr Col(int r, int g)
			requires (std::is_floating_point_v<T>)
		: m_R(std::max(r / 255.0f, 0.0f)), m_G(std::max(g / 255.0f, 0.0f)) {}

		constexpr Col(const Col&) = default;
		Col(Col&&) noexcept = default;

		static inline constexpr Col Zero() { return { 0, 0 }; }
		static inline constexpr Col Max()
		{
			return
			{
				MAX_CHANNEL_VALUE, MAX_CHANNEL_VALUE,
			};
		}

		constexpr Col GetR() const { return Col(m_R, 0); }
		constexpr Col GetG() const { return Col(0, m_G); }

		constexpr Vec<T, 2> AsVec() const { return Vec<T, 2>(m_R, m_G); }
		constexpr Vec<float, 2> AsHDR() const requires (!std::is_floating_point_v<T>) { return Vec<float, 2>(m_R, m_G); }

		explicit operator Vec<T, 2>() const { return AsVec(); }

		/// <summary>
		/// Returns a pointer to the first value in coltor, 
		/// allowing user to treat coltor components as array due to 
		/// them being packed right after one another in memory.
		/// </summary>
		/// <returns></returns>
		const T* GetMemPointer() const { return &m_R; }

		constexpr Col<float, 2> GetNormalized() const
			requires (!std::is_floating_point_v<T>)
		{
			return Col(float(m_R) / MAX_CHANNEL_VALUE, float(m_G) / MAX_CHANNEL_VALUE);
		}

		constexpr bool HasNonzeroRGB() const { return m_R > 0.0f || m_G > 0.0f; }

		std::string ToString(const std::uint8_t& decimalPlaces = 5) const requires std::is_floating_point_v<T>
		{
			return std::format("(Col {},{})",
				Utils::ToString(::Math::Roundf(m_R, decimalPlaces), decimalPlaces),
				Utils::ToString(::Math::Roundf(m_G, decimalPlaces), decimalPlaces));
		}
		std::string ToString() const requires std::is_same_v<T, std::uint8_t>
		{
			return std::format("[Col {},{}]", m_R, m_G);
		}

		T& operator[](const size_t index)
		{
			ENGINE_ASSERT(index <= 1, "Invalid Col index:{}", index);
			return m_Channels[index];
		}
		const T& operator[](const size_t index) const
		{
			ENGINE_ASSERT(index <= 1, "Invalid Col index:{}", index);
			return m_Channels[index];
		}

		Col& operator=(const Col& other) = default;
		Col& operator=(Col&& other) noexcept = default;
	};

	template<typename T, size_t ALIGN_BYTES>
		requires (std::is_arithmetic_v<T>)
	class alignas(ALIGN_BYTES) Col<T, 3, ALIGN_BYTES>
	{
	private:
	public:
		union
		{
			struct { T m_R, m_G, m_B; };
			T m_Channels[3];
		};

		static constexpr T MAX_CHANNEL_VALUE = std::numeric_limits<T>::max();

	public:
		constexpr Col()
			: m_R(), m_G(), m_B() {}

		constexpr Col(const std::array<T, 3>& arr)
			: Col(arr[0], arr[1], arr[2]) {}

		constexpr Col(const T r, const T g, const T b)
			: m_R(r), m_G(g), m_B(b) {}

		constexpr Col(const Vec<T, 2>& rg, const T b)
			: Col(rg.m_X, rg.m_Y, b) {}

		constexpr Col(const T r, const Vec<T, 2>& gb)
			: Col(r, gb.m_X, gb.m_Y) {}

		constexpr Col(const Vec<T, 3>& rgb)
			: Col(rgb.m_X, rgb.m_Y, rgb.m_Z) {}

		constexpr Col(const Col& rg, const T b)
			: Col(rg.m_R, rg.m_G, b) {}

		constexpr Col(const T r, const Col& gb)
			: Col(r, gb.m_G, gb.m_B) {}

		// ------------------------------------ FLOATING POINT ONLY CONSTRUCTORS --------------------------------
		constexpr Col(std::uint8_t r, std::uint8_t g, std::uint8_t b)
			requires (std::is_floating_point_v<T>)
		: m_R(r / 255.0f), m_G(g / 255.0f), m_B(b / 255.0f) {}

		constexpr Col(int r, int g, int b)
			requires (std::is_floating_point_v<T>)
		: m_R(std::max(r / 255.0f, 0.0f)), m_G(std::max(g / 255.0f, 0.0f)), m_B(std::max(b / 255.0f, 0.0f)) {}

		constexpr Col(const Col&) = default;
		Col(Col&&) noexcept = default;

		static inline constexpr Col Zero() { return { 0, 0, 0}; }
		static inline constexpr Col Max()
		{
			return
			{
				MAX_CHANNEL_VALUE, MAX_CHANNEL_VALUE,
				MAX_CHANNEL_VALUE
			};
		}

		constexpr Col GetR() const { return Col(m_R, 0, 0); }
		constexpr Col GetG() const { return Col(0, m_G, 0); }
		constexpr Col GetB() const { return Col(0, 0, m_B); }

		constexpr Col<T, 2> GetRG() const { return Col<T, 2>(m_R, m_G); }
		constexpr Col<T, 2> GetGB() const { return Col<T, 2>(m_G, m_B); }

		constexpr Vec<T, 3> AsVec() const { return Vec<T, 3>(m_R, m_G, m_B); }
		constexpr Vec<float, 3> AsHDR() const requires (!std::is_floating_point_v<T>) { return Vec<float, 3>(m_R, m_G, m_B); }

		constexpr explicit operator Vec<T, 3>() const { return AsVec(); }

		/// <summary>
		/// Returns a pointer to the first value in coltor, 
		/// allowing user to treat coltor components as array due to 
		/// them being packed right after one another in memory.
		/// </summary>
		/// <returns></returns>
		const T* GetMemPointer() const { return &m_R; }

		constexpr Col<float, 3> GetNormalized() const
			requires (!std::is_floating_point_v<T>)
		{
			return Col(float(m_R) / MAX_CHANNEL_VALUE, float(m_G) / MAX_CHANNEL_VALUE,
				float(m_B) / MAX_CHANNEL_VALUE);
		}

		constexpr bool HasNonzeroRGB() const { return m_R > 0.0f || m_G > 0.0f || m_B > 0.0f; }

		std::string ToString(const std::uint8_t& decimalPlaces = 5) const requires std::is_floating_point_v<T>
		{
			return std::format("(Col {},{},{})",
				Utils::ToString(::Math::Roundf(m_R, decimalPlaces), decimalPlaces),
				Utils::ToString(::Math::Roundf(m_G, decimalPlaces), decimalPlaces),
				Utils::ToString(::Math::Roundf(m_B, decimalPlaces), decimalPlaces));
		}
		std::string ToString() const requires std::is_same_v<T, std::uint8_t>
		{
			return std::format("[Col {},{},{}]", m_R, m_G, m_B);
		}

		T& operator[](const size_t index)
		{
			ENGINE_ASSERT(index <= 2, "Invalid Col index:{}", index);
			return m_Channels[index];
		}
		const T& operator[](const size_t index) const
		{
			ENGINE_ASSERT(index <= 2, "Invalid Col index:{}", index);
			return m_Channels[index];
		}

		Col& operator=(const Col& other) = default;
		Col& operator=(Col&& other) noexcept = default;
	};

	template<typename T, size_t ALIGN_BYTES>
		requires (std::is_arithmetic_v<T>)
	class alignas(ALIGN_BYTES) Col<T, 4, ALIGN_BYTES>
	{
	private:
	public:
		union
		{
			struct { T m_R, m_G, m_B, m_A; };
			T m_Channels[4];
		};

		static constexpr T MAX_CHANNEL_VALUE = std::numeric_limits<T>::max();

	public:
		constexpr Col(const std::array<T, 4>& arr)
			: Col(arr[0], arr[1], arr[2], arr[3]) {}

		constexpr Col(const Vec<T, 2>& rg, const T b, const T a)
			: Col(rg.m_X, rg.m_Y, b, a) {}

		constexpr Col(const T r, const Vec<T, 2>& gb, const T a)
			: Col(r, gb.m_X, gb.m_Y, a) {}

		constexpr Col(const T r, const T g, const Vec<T, 2>& ba)
			: Col(r, g, ba.m_X, ba.m_Y) {}

		constexpr Col(const Vec<T, 3>& rgb, const T a)
			: Col(rgb.m_X, rgb.m_Y, rgb.m_Z, a) {}

		constexpr Col(const T r, const Vec<T, 3>& gba)
			: Col(r, gba.m_X, gba.m_Y, gba.m_Z) {}

		constexpr Col(const Vec<T, 4>& rgba)
			: Col(rgba.m_X, rgba.m_Y, rgba.m_Z, rgba.m_W) {}

		constexpr Col(const Col& rg, const T b, const T a)
			: Col(rg.m_R, rg.m_G, b, a) {}

		constexpr Col(const T r, const Col& gb, const T a)
			: Col(r, gb.m_G, gb.m_B, a) {}

		constexpr Col(const T r, const T g, const Col& ba)
			: Col(r, g, ba.m_B, ba.m_A) {}

		constexpr Col(const Col& rgb, const T a)
			: Col(rgb.m_R, rgb.m_G, rgb.m_B, a) {}

		constexpr Col(const T r, const Col& gba)
			: Col(r, gba.m_G, gba.m_B, gba.m_A) {}

		// ---------------------------------------- UINT8 ONLY CONSTRUCTORS -------------------------------------
		constexpr Col() requires std::is_same_v<T, std::uint8_t>
			: m_R(0), m_G(0), m_B(0), m_A(MAX_CHANNEL_VALUE) {}

		constexpr Col(std::uint8_t r, std::uint8_t g, std::uint8_t b) requires std::is_same_v<T, std::uint8_t>
			: m_R(r), m_G(g), m_B(b), m_A(MAX_CHANNEL_VALUE) {}

		constexpr Col(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a) requires std::is_same_v<T, std::uint8_t>
			: m_R(r), m_G(g), m_B(b), m_A(a) {}


		// ------------------------------------ FLOATING POINT ONLY CONSTRUCTORS --------------------------------
		constexpr Col() requires std::is_floating_point_v<T>
			: m_R(0.0f), m_G(0.0f), m_B(0.0f), m_A(1.0f) {}

		constexpr Col(const T r, const T g, const T b)
			requires std::is_floating_point_v<T>
		: m_R(r), m_G(g), m_B(b), m_A(1.0f) {}

		constexpr Col(const T r, const T g, const T b, const T a)
			requires std::is_floating_point_v<T>
		: m_R(r), m_G(g), m_B(b), m_A(std::min(a, T(1))) {}

		constexpr Col(std::uint8_t r, std::uint8_t g, std::uint8_t b)
			requires (std::is_floating_point_v<T>)
		: m_R(r / 255.0f), m_G(g / 255.0f), m_B(b / 255.0f), m_A(1.0f) {}

		constexpr Col(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a)
			requires (std::is_floating_point_v<T>)
		: m_R(r / 255.0f), m_G(g / 255.0f), m_B(b / 255.0f), m_A(a / 255.0f) {}

		constexpr Col(int r, int g, int b)
			requires (std::is_floating_point_v<T>)
		: m_R(std::max(r / 255.0f, 0.0f)), m_G(std::max(g / 255.0f, 0.0f)), m_B(std::max(b / 255.0f, 0.0f)), m_A(1.0f) {}

		constexpr Col(int r, int g, int b, int a)
			requires (std::is_floating_point_v<T>)
		: m_R(std::max(r / 255.0f, 0.0f)), m_G(std::max(g / 255.0f, 0.0f)),
			m_B(std::max(b / 255.0f, 0.0f)), m_A(std::max(a / 255.0f, 0.0f)) {}

		constexpr Col(const Col&) = default;
		Col(Col&&) noexcept = default;

		static inline constexpr Col Zero() { return { 0, 0, 0, 0 }; }
		static inline constexpr Col Max()
		{
			return
			{
				MAX_CHANNEL_VALUE, MAX_CHANNEL_VALUE,
				MAX_CHANNEL_VALUE, MAX_CHANNEL_VALUE
			};
		}

		constexpr Col GetR() const { return Col(m_R, 0, 0, 0); }
		constexpr Col GetG() const { return Col(0, m_G, 0, 0); }
		constexpr Col GetB() const { return Col(0, 0, m_B, 0); }
		constexpr Col GetA() const { return Col(0, 0, 0, m_A); }

		constexpr Col<T, 2> GetRG() const { return Col<T, 2>(m_R, m_G); }
		constexpr Col<T, 2> GetGB() const { return Col<T, 2>(m_G, m_B); }
		constexpr Col<T, 2> GetBA() const { return Col<T, 2>(m_B, m_A); }

		constexpr Col<T, 3> GetRGB() const { return Col<T, 3>(m_R, m_G, m_B); }
		constexpr Col<T, 3> GetGBA() const { return Col<T, 3>(m_G, m_B, m_A); }
		constexpr Vec<T, 4> AsVec() const { return Vec<T, 4>(m_R, m_G, m_B, m_A); }
		constexpr Vec<float, 4> AsHDR() const requires (!std::is_floating_point_v<T>) { return Vec<float, 4>(m_R, m_G, m_B, m_A); }

		constexpr explicit operator Vec<T, 4>() const { return AsVec(); }

		/// <summary>
		/// Returns a pointer to the first value in coltor, 
		/// allowing user to treat coltor components as array due to 
		/// them being packed right after one another in memory.
		/// </summary>
		/// <returns></returns>
		const T* GetMemPointer() const { return &m_R; }

		constexpr Col<float, 4> GetNormalized() const
			requires (!std::is_floating_point_v<T>)
		{
			return Col(float(m_R) / MAX_CHANNEL_VALUE, float(m_G) / MAX_CHANNEL_VALUE,
				float(m_B) / MAX_CHANNEL_VALUE, float(m_A) / MAX_CHANNEL_VALUE);
		}

		constexpr bool HasNonzeroRGB() const { return m_R > 0.0f || m_G > 0.0f || m_B > 0.0f; }
		constexpr bool HasNonzeroAlpha() const { return m_A > 0.0f; }
		constexpr bool HasVisibleNonzeroRGB() const
		{
			return HasNonzeroRGB() && HasNonzeroAlpha();
		}

		std::string ToString(const std::uint8_t& decimalPlaces = 5) const requires std::is_floating_point_v<T>
		{
			return std::format("(Col {},{},{},{})",
				Utils::ToString(::Math::Roundf(m_R, decimalPlaces), decimalPlaces),
				Utils::ToString(::Math::Roundf(m_G, decimalPlaces), decimalPlaces),
				Utils::ToString(::Math::Roundf(m_B, decimalPlaces), decimalPlaces),
				Utils::ToString(::Math::Roundf(m_A, decimalPlaces), decimalPlaces));
		}
		std::string ToString() const requires std::is_same_v<T, std::uint8_t>
		{
			return std::format("[Col {},{},{},{}]", m_R, m_G, m_B, m_A);
		}

		T& operator[](const size_t index)
		{
			ENGINE_ASSERT(index <= 3, "Invalid Col index:{}", index);
			return m_Channels[index];
		}
		const T& operator[](const size_t index) const
		{
			ENGINE_ASSERT(index <= 3, "Invalid Col index:{}", index);
			return m_Channels[index];
		}

		Col& operator=(const Col& other) = default;
		Col& operator=(Col&& other) noexcept = default;
	};

	template<typename TLhs, size_t LHS_ALIGN, size_t N, typename TRhs, size_t RHS_ALIGN>
	constexpr auto operator+(const Col<TLhs, N, LHS_ALIGN>& lhs, const Col<TRhs, N, RHS_ALIGN>& rhs)
		-> Col<std::common_type_t<TLhs, TRhs>, N, LHS_ALIGN>
	{
		using CommonType = std::common_type_t<TLhs, TRhs>;
		Col<CommonType, N, LHS_ALIGN> result = lhs;
		for (size_t i = 0; i < N; i++) result.m_Channels[i] += rhs.m_Channels[i];
		return result;
	}


	template<typename TLhs, size_t LHS_ALIGN, size_t N, typename TRhs, size_t RHS_ALIGN>
	constexpr auto operator-(const Col<TLhs, N, LHS_ALIGN>& lhs, const Col<TRhs, N, RHS_ALIGN>& rhs)
		-> Col<std::common_type_t<TLhs, TRhs>, N, LHS_ALIGN>
	{
		using CommonType = std::common_type_t<TLhs, TRhs>;
		Col<CommonType, N, LHS_ALIGN> result = lhs;
		for (size_t i = 0; i < N; i++) result.m_Channels[i] -= rhs.m_Channels[i];
		return result;
	}


	template<typename TLhs, size_t LHS_ALIGN, size_t N, typename TRhs, size_t RHS_ALIGN>
	constexpr auto operator*(const Col<TLhs, N, LHS_ALIGN>& lhs, const Col<TRhs, N, RHS_ALIGN>& rhs)
		-> Col<std::common_type_t<TLhs, TRhs>, N, LHS_ALIGN>
	{
		using CommonType = std::common_type_t<TLhs, TRhs>;
		Col<CommonType, N, LHS_ALIGN> result = lhs;
		for (size_t i = 0; i < N; i++) result.m_Channels[i] *= rhs.m_Channels[i];
		return result;
	}

	template<typename TScalar, typename TCol, size_t N, size_t ALIGN>
		requires (std::is_arithmetic_v<TScalar>)
	constexpr auto operator*(TScalar lhs, const Col<TCol, N, ALIGN>& rhs)
		-> Col<std::common_type_t<TScalar, TCol>, N, ALIGN>
	{
		using CommonType = std::common_type_t<TScalar, TCol>;
		Col<CommonType, N, ALIGN> result = rhs;
		for (size_t i = 0; i < N; i++) result.m_Channels[i] *= lhs;
		return result;
	}

	template<typename TScalar, typename TCol, size_t N, size_t ALIGN>
		requires (std::is_arithmetic_v<TScalar>)
	constexpr auto operator*(const Col<TCol, N, ALIGN>& lhs, TScalar rhs)
		-> Col<std::common_type_t<TScalar, TCol>, N, ALIGN>
	{
		using CommonType = std::common_type_t<TScalar, TCol>;
		Col<CommonType, N, ALIGN> result = lhs;
		for (size_t i = 0; i < N; i++) result.m_Channels[i] *= rhs;
		return result;
	}


	template<typename TLhs, size_t LHS_ALIGN, size_t N, typename TRhs, size_t RHS_ALIGN>
	constexpr auto operator/(const Col<TLhs, N, LHS_ALIGN>& lhs, const Col<TRhs, N, RHS_ALIGN>& rhs)
		-> Col<std::common_type_t<TLhs, TRhs>, N, LHS_ALIGN>
	{
		using CommonType = std::common_type_t<TLhs, TRhs>;
		Col<CommonType, N, LHS_ALIGN> result = lhs;
		for (size_t i = 0; i < N; i++)
		{
			if constexpr (std::is_floating_point_v<TRhs>)
			{
				ENGINE_ASSERT(!::Math::ApproximateEqualsF(rhs.m_Channels[i], 0),
					"Tried to divide a Coltor:{} by a 0-value Coltor component: {}", lhs.ToString(), rhs.ToString());
			}
			else
			{
				ENGINE_ASSERT(rhs.m_Channels[i] != 0,
					"Tried to divide a Coltor:{} by a 0-value Coltor component: {}", lhs.ToString(), rhs.ToString());
			}

			result.m_Channels[i] /= rhs.m_Channels[i];
		}
		return result;
	}

	template<typename TScalar, typename TCol, size_t N, size_t ALIGN>
		requires (std::is_arithmetic_v<TScalar>)
	constexpr auto operator/(const Col<TCol, N, ALIGN>& lhs, TScalar rhs)
		-> Col<std::common_type_t<TScalar, TCol>, N, ALIGN>
	{
		if constexpr (std::is_floating_point_v<TScalar>)
		{
			ENGINE_ASSERT(!::Math::ApproximateEqualsF(rhs, 0),
				"Tried to divide a Coltor:{} by a 0-value scalar: {}", lhs.ToString(), rhs);
		}
		else
		{
			ENGINE_ASSERT(rhs != 0, "Tried to divide a Coltor:{} by a 0-value scalar: {}", lhs.ToString(), rhs);
		}

		using CommonType = std::common_type_t<TScalar, TCol>;
		Col<CommonType, N, ALIGN> result = lhs;
		for (size_t i = 0; i < N; i++) result.m_Channels[i] /= rhs;
		return result;
	}

	template<typename TLhs, size_t LHS_ALIGN, size_t N, typename TRhs, size_t RHS_ALIGN>
	bool operator==(const Col<TLhs, N, LHS_ALIGN>& lhs, const Col<TRhs, N, RHS_ALIGN>& rhs)
	{
		for (size_t i = 0; i < N; i++)
		{
			if (!::Math::ApproximateEqualsF(lhs.m_Channels[i], rhs.m_Channels[i]))
				return false;
		}
		return true;
	}
	template<typename TLhs, size_t LHS_ALIGN, size_t N, typename TRhs, size_t RHS_ALIGN>
	bool operator!=(const Col<TLhs, N, LHS_ALIGN>& lhs, const Col<TRhs, N, RHS_ALIGN>& rhs)
	{
		return !(lhs == rhs);
	}
	template<typename TLhs, size_t LHS_ALIGN, size_t N, typename TRhs, size_t RHS_ALIGN>
	constexpr bool operator>(const Col<TLhs, N, LHS_ALIGN>& lhs, const Col<TRhs, N, RHS_ALIGN>& rhs)
	{
		for (size_t i = 0; i < N; i++)
		{
			if (lhs.m_Channels[i] <= rhs.m_Channels[i])
				return false;
		}
		return true;
	}
	template<typename TLhs, size_t LHS_ALIGN, size_t N, typename TRhs, size_t RHS_ALIGN>
	bool operator>=(const Col<TLhs, N, LHS_ALIGN>& lhs, const Col<TRhs, N, RHS_ALIGN>& rhs)
	{
		return (lhs > rhs || lhs == rhs);
	}
	template<typename TLhs, size_t LHS_ALIGN, size_t N, typename TRhs, size_t RHS_ALIGN>
	bool operator<(const Col<TLhs, N, LHS_ALIGN>& lhs, const Col<TRhs, N, RHS_ALIGN>& rhs)
	{
		for (size_t i = 0; i < N; i++)
		{
			if (lhs.m_Channels[i] >= rhs.m_Channels[i])
				return false;
		}
		return true;
	}
	template<typename TLhs, size_t LHS_ALIGN, size_t N, typename TRhs, size_t RHS_ALIGN>
	bool operator<=(const Col<TLhs, N, LHS_ALIGN>& lhs, const Col<TRhs, N, RHS_ALIGN>& rhs)
	{
		return (lhs < rhs || lhs == rhs);
	}
}