/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#ifndef GP_TEST_FRAMEWORK_H
#define GP_TEST_FRAMEWORK_H

/*
 * A deliberately tiny host-side test runner.
 *
 * The GBA build cannot run a normal test framework, so the pure-logic half of the game
 * (gp::board, gp::cursor, gp::tile_kind) is compiled for the host instead and exercised here.
 * This has no third party dependencies on purpose: it keeps CI hermetic and the build instant.
 * Swap it for doctest or Catch2 if the suite ever outgrows it.
 */

#include <cstdio>
#include <string>
#include <vector>
#include <type_traits>

namespace gp_test
{

using test_function = void (*)();

struct test_case
{
    const char* name;
    test_function function;
};

inline std::vector<test_case>& test_cases()
{
    static std::vector<test_case> instance;
    return instance;
}

inline int& failure_count()
{
    static int instance = 0;
    return instance;
}

inline int& check_count()
{
    static int instance = 0;
    return instance;
}

struct registrar
{
    registrar(const char* name, test_function function)
    {
        test_cases().push_back(test_case{name, function});
    }
};

template<typename Type>
std::string to_text(const Type& value)
{
    if constexpr (std::is_enum_v<Type>)
    {
        return std::to_string(static_cast<long long>(value));
    }
    else if constexpr (std::is_arithmetic_v<Type>)
    {
        return std::to_string(value);
    }
    else
    {
        return std::string("<value>");
    }
}

inline void report(const char* file, int line, const char* expression, const std::string& detail)
{
    ++failure_count();
    std::printf("    FAIL %s:%d\n         %s\n", file, line, expression);

    if (!detail.empty())
    {
        std::printf("         %s\n", detail.c_str());
    }
}

inline int run_all()
{
    const std::vector<test_case>& cases = test_cases();
    int failed_cases = 0;

    std::printf("running %zu test cases\n", cases.size());

    for (const test_case& current : cases)
    {
        int before = failure_count();
        current.function();
        bool passed = failure_count() == before;

        if (!passed)
        {
            ++failed_cases;
        }

        std::printf("  %s %s\n", passed ? "ok  " : "FAIL", current.name);
    }

    std::printf("\n%d checks, %d failed, %d/%zu cases passed\n", check_count(), failure_count(),
                int(cases.size()) - failed_cases, cases.size());

    return failed_cases == 0 ? 0 : 1;
}

} // namespace gp_test

#define GP_TEST(test_name)                                                                                             \
    static void test_name();                                                                                           \
    static ::gp_test::registrar test_name##_registrar(#test_name, &test_name);                                         \
    static void test_name()

#define GP_CHECK(expression)                                                                                           \
    do                                                                                                                 \
    {                                                                                                                  \
        ++::gp_test::check_count();                                                                                    \
                                                                                                                       \
        if (!(expression))                                                                                             \
        {                                                                                                              \
            ::gp_test::report(__FILE__, __LINE__, #expression, std::string());                                         \
        }                                                                                                              \
    } while (false)

#define GP_CHECK_EQ(actual, expected)                                                                                  \
    do                                                                                                                 \
    {                                                                                                                  \
        ++::gp_test::check_count();                                                                                    \
                                                                                                                       \
        auto&& gp_actual = (actual);                                                                                   \
        auto&& gp_expected = (expected);                                                                               \
                                                                                                                       \
        if (!(gp_actual == gp_expected))                                                                               \
        {                                                                                                              \
            ::gp_test::report(__FILE__, __LINE__, #actual " == " #expected,                                            \
                              "actual: " + ::gp_test::to_text(gp_actual) +                                             \
                                      "  expected: " + ::gp_test::to_text(gp_expected));                               \
        }                                                                                                              \
    } while (false)

#endif
