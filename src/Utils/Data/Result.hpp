#pragma once
#include <cstdint>
#include <string>
#include <new>
#include <utility>

enum class ResultState : std::uint8_t
{
	Value = 0,
	Error = 1
};
template<typename ResultT, typename ErrorT>
class Result
{
private:
	//TODO: replace with union -> note: non intrinsic types like std::string require manual management of creation/deletion itn
	//std::variant<ResultT, ErrorT> m_result;
	ResultState m_state;

	union ResultUnion
	{
		ResultT m_Value;
		ErrorT m_Error;

		ResultUnion() {}
		ResultUnion(const ResultT& value) : m_Value(value) {}
		ResultUnion(const ErrorT& error) : m_Error(error) {}
		~ResultUnion() {}
	};
	ResultUnion m_result;
public:

private:
	void DestroyCurrent()
	{
		if (m_state == ResultState::Value)
			m_result.m_Value.~ResultT();
		else
			m_result.m_Error.~ErrorT();
	}

public:
	Result(const ResultT& result) : m_result(result), m_state(ResultState::Value) {}
	Result(const ErrorT& error) : m_result(error), m_state(ResultState::Error) {}

	Result(const Result& other) : m_result(), m_state(other.m_state)
	{
		if (m_state == ResultState::Value)
			new (&m_result.m_Value) ResultT(other.m_result.m_Value);
		else
			new (&m_result.m_Error) ErrorT(other.m_result.m_Error);
	}

	Result(Result&& other) noexcept : m_state(other.m_state)
	{
		if (m_state == ResultState::Value)
			new (&m_result.m_Value) ResultT(std::move(other.m_result.m_Value));
		else
			new (&m_result.m_Error) ErrorT(std::move(other.m_result.m_Error));
	}

	~Result()
	{
		DestroyCurrent();
	}

	bool HasValue() const
	{
		return m_state == ResultState::Value;
	}
	bool HasError() const
	{
		return m_state == ResultState::Error;
	}

	const ResultT& GetValue(const ResultT& defaultVal) const
	{
		if (m_state == ResultState::Error)
			return defaultVal;
		return m_result.m_Value;
	}
	const ResultT* TryGetValue() const
	{
		if (m_state == ResultState::Error)
			return nullptr;
		return &m_result.m_Value;
	}

	const ErrorT& GetError(const ErrorT& defaultVal) const
	{
		if (m_state == ResultState::Value)
			return defaultVal;
		return m_result.m_Error;
	}
	const ErrorT* TryGetError() const
	{
		if (m_state == ResultState::Value)
			return nullptr;
		return &m_result.m_Error;
	}

	Result& operator=(const Result& other)
	{
		if (*this != other)
		{
			//For union to work properly with non-trivial types/construction/destrctions like std::string
			//we must destroy the old value to not leave any resources behind
			DestroyCurrent();

			m_state = other.m_state;
			//We must then emplace the new value with placement new, allowing us to create memory
			//in place in a specific memory location. This is neccessary because of non-trivial constructors
			if (m_result == ResultState::Value)
			{
				new (&m_result.m_Value) ResultT(other.m_result.m_Value);
			}
			else if (m_result == ResultState::Error)
			{
				new (&m_result.m_Error) ErrorT(other.m_result.m_Error);
			}
		}
		return *this;
	}
	Result& operator=(Result&& other) noexcept
	{
		if (*this != other)
		{
			DestroyCurrent();

			m_state = other.m_state;
			if (m_result == ResultState::Value)
			{
				new (&m_result.m_Value) ResultT(std::move(other.m_result.m_Value));
			}
			else if (m_result == ResultState::Error)
			{
				new (&m_result.m_Error) ErrorT(std::move(other.m_result.m_Error));
			}
		}
		return *this;
	}
};

template<typename ResultT>
using BasicResult = Result<ResultT, std::string>;
