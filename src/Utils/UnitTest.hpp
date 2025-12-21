#pragma once
#include <functional>
#include <tuple>
#include <vector>
#include <optional>
#include <iostream>
#include "AnsiCodes.hpp"
#include "Utils/TemplateConcepts.hpp"
#include "Utils/ToStringFunctions.hpp"
#include "Utils/Debug.hpp"

namespace UnitTest
{
	constexpr const char* TEST_LOG_ANSI_COLOR = ANSI_COLOR_BLUE;
	constexpr const char* TEST_ERROR_ANSI_COLOR = ANSI_COLOR_RED;
	constexpr const char* TEST_SUCCESS_ANSI_COLOR = ANSI_COLOR_GREEN;

	template<typename TReturn, typename... TArgs>
	struct FunctionTest
	{
		std::tuple<TArgs...> m_Input;
		TReturn m_ExpectedOutput;

		FunctionTest(const TReturn& returnVal, TArgs&&... args)
			: m_ExpectedOutput(returnVal), m_Input(std::make_tuple(args...)) {}
	};

	template<typename TReturn, typename... TArgs>
	bool TestFunction(const char* functionName, const std::function<TReturn(TArgs...)>& testFunc, 
		const std::vector<FunctionTest<TReturn, TArgs...>>& tests)
	{
		int totalPassed = 0;
		std::cout << std::format("{}[{}UNIT TEST @{}{}]{} Running {} test(s) for '{}'...", ANSI_COLOR_WHITE, ANSI_COLOR_GRAY, FormatCurrentTime(), 
			ANSI_COLOR_WHITE, TEST_LOG_ANSI_COLOR, tests.size(), functionName) << std::endl;
		for (size_t i =0; i<tests.size(); i++)
		{
			try
			{
				TReturn testResult = std::apply(testFunc, tests[i].m_Input);
				if (tests[i].m_ExpectedOutput != testResult)
				{
					LogFailedTest(i + 1, tests.size(), tests[i], testResult);
				}
				else
				{
					totalPassed++;
					LogPassedTest(i + 1, tests.size(), tests[i]);
				}
			}
			catch (const std::exception& unexpectedException)
			{
				LogFailedTest(i + 1, tests.size(), tests[i], unexpectedException);
			}
		}

		std::cout << std::format("{}[{}UNIT TEST @{}{}]{} Finished tests with {}/{} passed", ANSI_COLOR_WHITE, ANSI_COLOR_GRAY, FormatCurrentTime(),
			ANSI_COLOR_WHITE, TEST_LOG_ANSI_COLOR, totalPassed, tests.size()) << std::endl;
		return totalPassed == tests.size();
	}

	template<typename TReturn, typename... TArgs>
	void LogPassedTest(const std::uint16_t testNumber, const std::uint16_t totalTests, const FunctionTest<TReturn, TArgs...>& test)
	{
		std::cout << std::format("{} {}[{}{}/{}{}]{} Successfully passed test ({}) -> {}", FormatCurrentTime(), ANSI_COLOR_WHITE, ANSI_COLOR_GRAY, 
			testNumber, totalTests, ANSI_COLOR_WHITE, TEST_SUCCESS_ANSI_COLOR, 
			Utils::ToStringTuple(test.m_Input), Utils::ToStringForced(test.m_ExpectedOutput)) << std::endl;
	}
	template<typename TReturn, typename TResult, typename... TArgs>
	void LogFailedTest(const std::uint16_t testNumber, const std::uint16_t totalTests, 
		const FunctionTest<TReturn, TArgs...>& test, const TResult& receivedOutput)
	{
		std::cout << std::format("{} {}[{}{}/{}{}]{} Failed test ({}) -> {} by getting {}", FormatCurrentTime(), ANSI_COLOR_WHITE, ANSI_COLOR_GRAY,
			testNumber, totalTests, ANSI_COLOR_WHITE, TEST_ERROR_ANSI_COLOR,
			Utils::ToStringTuple(test.m_Input), Utils::ToStringForced(test.m_ExpectedOutput), 
			(Utils::IsExceptionType<TResult>? std::format("{} exception: {}", typeid(TResult).name(), Utils::ToString(receivedOutput))
				: std::format("output: {}", Utils::ToStringForced(receivedOutput)))) << std::endl;
	}
}