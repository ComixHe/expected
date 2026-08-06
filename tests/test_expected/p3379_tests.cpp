#include <catch2/catch_all.hpp>

#include <zeus/expected.hpp>

#include <type_traits>
#include <utility>

using namespace zeus;

namespace
{

template<class Lhs, class Rhs, class = void>
inline constexpr bool has_equal_to_v = false;

template<class Lhs, class Rhs>
inline constexpr bool has_equal_to_v<Lhs, Rhs, std::void_t<decltype(std::declval<const Lhs &>() == std::declval<const Rhs &>())>> = true;

template<class Lhs, class Rhs, class = void>
inline constexpr bool has_not_equal_to_v = false;

template<class Lhs, class Rhs>
inline constexpr bool has_not_equal_to_v<Lhs, Rhs, std::void_t<decltype(std::declval<const Lhs &>() != std::declval<const Rhs &>())>> =
    true;

struct NotBoolean
{
};

struct NonBooleanComparable
{
};

NotBoolean operator==(const NonBooleanComparable &, const NonBooleanComparable &)
{
    return {};
}

struct ImplicitBoolean
{
    bool value;

    constexpr operator bool() const noexcept { return value; }
    explicit  operator bool() = delete;
};

struct ImplicitlyBooleanComparable
{
};

ImplicitBoolean operator==(const ImplicitlyBooleanComparable &, const ImplicitlyBooleanComparable &)
{
    return {true};
}

struct OtherError
{
};

using ExpectedOperand = expected<int, OtherError>;

struct ComparesWithExpected
{
};

bool operator==(const ComparesWithExpected &, const ExpectedOperand &)
{
    return true;
}

struct ValueError
{
};

} // namespace

TEST_CASE("P3379R0 constrains expected-to-expected comparison", "[P3379R0][equality]")
{
    using ValidComparison       = expected<int, int>;
    using InvalidValueResult    = expected<NonBooleanComparable, int>;
    using InvalidErrorResult    = expected<int, NonBooleanComparable>;
    using ImplicitlyConvertible = expected<ImplicitlyBooleanComparable, int>;

    STATIC_REQUIRE(has_equal_to_v<ValidComparison, ValidComparison>);
    STATIC_REQUIRE(has_not_equal_to_v<ValidComparison, ValidComparison>);
    STATIC_REQUIRE_FALSE(has_equal_to_v<InvalidValueResult, InvalidValueResult>);
    STATIC_REQUIRE_FALSE(has_not_equal_to_v<InvalidValueResult, InvalidValueResult>);
    STATIC_REQUIRE_FALSE(has_equal_to_v<InvalidErrorResult, InvalidErrorResult>);
    STATIC_REQUIRE_FALSE(has_not_equal_to_v<InvalidErrorResult, InvalidErrorResult>);
    STATIC_REQUIRE(has_equal_to_v<ImplicitlyConvertible, ImplicitlyConvertible>);
}

TEST_CASE("P3379R0 constrains both orders of heterogeneous expected comparisons", "[P3379R0][equality]")
{
    using ValidComparison    = expected<int, int>;
    using InvalidValueResult = expected<NonBooleanComparable, int>;
    using InvalidErrorResult = expected<int, NonBooleanComparable>;

    STATIC_REQUIRE_FALSE(has_equal_to_v<ValidComparison, InvalidValueResult>);
    STATIC_REQUIRE_FALSE(has_equal_to_v<InvalidValueResult, ValidComparison>);
    STATIC_REQUIRE_FALSE(has_not_equal_to_v<ValidComparison, InvalidValueResult>);
    STATIC_REQUIRE_FALSE(has_not_equal_to_v<InvalidValueResult, ValidComparison>);

    STATIC_REQUIRE_FALSE(has_equal_to_v<ValidComparison, InvalidErrorResult>);
    STATIC_REQUIRE_FALSE(has_equal_to_v<InvalidErrorResult, ValidComparison>);
    STATIC_REQUIRE_FALSE(has_not_equal_to_v<ValidComparison, InvalidErrorResult>);
    STATIC_REQUIRE_FALSE(has_not_equal_to_v<InvalidErrorResult, ValidComparison>);
}

TEST_CASE("P3379R0 constrains expected-to-value comparison", "[P3379R0][equality]")
{
    using InvalidValueResult = expected<NonBooleanComparable, int>;

    STATIC_REQUIRE(has_equal_to_v<expected<int, int>, int>);
    STATIC_REQUIRE_FALSE(has_equal_to_v<InvalidValueResult, NonBooleanComparable>);
    STATIC_REQUIRE_FALSE(has_equal_to_v<NonBooleanComparable, InvalidValueResult>);
    STATIC_REQUIRE_FALSE(has_not_equal_to_v<InvalidValueResult, NonBooleanComparable>);
    STATIC_REQUIRE_FALSE(has_not_equal_to_v<NonBooleanComparable, InvalidValueResult>);
}

TEST_CASE("P3379R0 prevents an expected operand from using the value overload", "[P3379R0][equality]")
{
    using Lhs = expected<ComparesWithExpected, ValueError>;

    STATIC_REQUIRE_FALSE(has_equal_to_v<Lhs, ExpectedOperand>);
    STATIC_REQUIRE_FALSE(has_not_equal_to_v<Lhs, ExpectedOperand>);
}

TEST_CASE("P3379R0 keeps nested expected comparisons valid", "[P3379R0][equality]")
{
    using Inner       = expected<int, int>;
    using NestedValue = expected<Inner, int>;
    using NestedError = expected<int, Inner>;

    STATIC_REQUIRE(has_equal_to_v<NestedValue, NestedValue>);
    STATIC_REQUIRE(has_not_equal_to_v<NestedValue, NestedValue>);
    STATIC_REQUIRE(has_equal_to_v<NestedError, NestedError>);
    STATIC_REQUIRE(has_not_equal_to_v<NestedError, NestedError>);
    STATIC_REQUIRE(has_equal_to_v<NestedError, zeus::unexpected<Inner>>);
    STATIC_REQUIRE(has_not_equal_to_v<NestedError, zeus::unexpected<Inner>>);

    STATIC_REQUIRE(has_equal_to_v<Inner, NestedValue>);
    STATIC_REQUIRE(has_equal_to_v<NestedValue, Inner>);
    STATIC_REQUIRE(has_not_equal_to_v<Inner, NestedValue>);
    STATIC_REQUIRE(has_not_equal_to_v<NestedValue, Inner>);
}

TEST_CASE("P3379R0 constrains expected-to-unexpected comparison", "[P3379R0][equality]")
{
    using InvalidErrorResult = expected<int, NonBooleanComparable>;
    using InvalidUnexpected  = zeus::unexpected<NonBooleanComparable>;

    STATIC_REQUIRE(has_equal_to_v<expected<int, int>, zeus::unexpected<int>>);
    STATIC_REQUIRE_FALSE(has_equal_to_v<InvalidErrorResult, InvalidUnexpected>);
    STATIC_REQUIRE_FALSE(has_not_equal_to_v<InvalidErrorResult, InvalidUnexpected>);
}

TEST_CASE("P3379R0 constrains expected<void, E> comparisons", "[P3379R0][equality]")
{
    using InvalidExpected   = expected<void, NonBooleanComparable>;
    using InvalidUnexpected = zeus::unexpected<NonBooleanComparable>;

    STATIC_REQUIRE(has_equal_to_v<expected<void, int>, expected<void, int>>);
    STATIC_REQUIRE_FALSE(has_equal_to_v<InvalidExpected, InvalidExpected>);
    STATIC_REQUIRE_FALSE(has_not_equal_to_v<InvalidExpected, InvalidExpected>);
    STATIC_REQUIRE_FALSE(has_equal_to_v<InvalidExpected, InvalidUnexpected>);
    STATIC_REQUIRE_FALSE(has_not_equal_to_v<InvalidExpected, InvalidUnexpected>);
}
