#include <catch2/catch_all.hpp>

#include <zeus/expected.hpp>

#include <utility>

namespace
{

template<bool IsNothrow>
struct BooleanConversion
{
    bool value;

    constexpr operator bool() const noexcept(IsNothrow) { return value; }
};

struct ThrowingConversionLhs
{
};

struct ThrowingConversionRhs
{
};

BooleanConversion<false> operator==(const ThrowingConversionLhs &, const ThrowingConversionRhs &) noexcept
{
    return {true};
}

struct NothrowConversionLhs
{
};

struct NothrowConversionRhs
{
};

BooleanConversion<true> operator==(const NothrowConversionLhs &, const NothrowConversionRhs &) noexcept
{
    return {true};
}

} // namespace

TEST_CASE("equality noexcept includes the implicit conversion to bool", "[noexcept][equality][LWG-4366]")
{
    using ObjectLhs        = zeus::expected<ThrowingConversionLhs, ThrowingConversionLhs>;
    using ObjectRhs        = zeus::expected<ThrowingConversionRhs, ThrowingConversionRhs>;
    using VoidLhs          = zeus::expected<void, ThrowingConversionLhs>;
    using VoidRhs          = zeus::expected<void, ThrowingConversionRhs>;
    using ObjectUnexpected = zeus::unexpected<ThrowingConversionRhs>;

    // These are the five primary equality overloads.
    STATIC_REQUIRE_FALSE(noexcept(std::declval<const ObjectLhs &>() == std::declval<const ObjectRhs &>()));
    STATIC_REQUIRE_FALSE(noexcept(std::declval<const ObjectLhs &>() == std::declval<const ThrowingConversionRhs &>()));
    STATIC_REQUIRE_FALSE(noexcept(std::declval<const ObjectLhs &>() == std::declval<const ObjectUnexpected &>()));
    STATIC_REQUIRE_FALSE(noexcept(std::declval<const VoidLhs &>() == std::declval<const VoidRhs &>()));
    STATIC_REQUIRE_FALSE(noexcept(std::declval<const VoidLhs &>() == std::declval<const ObjectUnexpected &>()));

    // These are explicit overloads in C++17 and rewritten equality candidates in C++20 and later.
    STATIC_REQUIRE_FALSE(noexcept(std::declval<const ObjectLhs &>() != std::declval<const ObjectRhs &>()));
    STATIC_REQUIRE_FALSE(noexcept(std::declval<const ThrowingConversionRhs &>() == std::declval<const ObjectLhs &>()));
    STATIC_REQUIRE_FALSE(noexcept(std::declval<const ObjectLhs &>() != std::declval<const ThrowingConversionRhs &>()));
    STATIC_REQUIRE_FALSE(noexcept(std::declval<const ThrowingConversionRhs &>() != std::declval<const ObjectLhs &>()));
    STATIC_REQUIRE_FALSE(noexcept(std::declval<const ObjectUnexpected &>() == std::declval<const ObjectLhs &>()));
    STATIC_REQUIRE_FALSE(noexcept(std::declval<const ObjectLhs &>() != std::declval<const ObjectUnexpected &>()));
    STATIC_REQUIRE_FALSE(noexcept(std::declval<const ObjectUnexpected &>() != std::declval<const ObjectLhs &>()));
    STATIC_REQUIRE_FALSE(noexcept(std::declval<const VoidLhs &>() != std::declval<const VoidRhs &>()));
    STATIC_REQUIRE_FALSE(noexcept(std::declval<const ObjectUnexpected &>() == std::declval<const VoidLhs &>()));
    STATIC_REQUIRE_FALSE(noexcept(std::declval<const VoidLhs &>() != std::declval<const ObjectUnexpected &>()));
    STATIC_REQUIRE_FALSE(noexcept(std::declval<const ObjectUnexpected &>() != std::declval<const VoidLhs &>()));
}

TEST_CASE("equality is noexcept when comparison and bool conversion are non-throwing", "[noexcept][equality]")
{
    using ObjectLhs        = zeus::expected<NothrowConversionLhs, NothrowConversionLhs>;
    using ObjectRhs        = zeus::expected<NothrowConversionRhs, NothrowConversionRhs>;
    using VoidLhs          = zeus::expected<void, NothrowConversionLhs>;
    using VoidRhs          = zeus::expected<void, NothrowConversionRhs>;
    using ObjectUnexpected = zeus::unexpected<NothrowConversionRhs>;

    // These are the five primary equality overloads.
    STATIC_REQUIRE(noexcept(std::declval<const ObjectLhs &>() == std::declval<const ObjectRhs &>()));
    STATIC_REQUIRE(noexcept(std::declval<const ObjectLhs &>() == std::declval<const NothrowConversionRhs &>()));
    STATIC_REQUIRE(noexcept(std::declval<const ObjectLhs &>() == std::declval<const ObjectUnexpected &>()));
    STATIC_REQUIRE(noexcept(std::declval<const VoidLhs &>() == std::declval<const VoidRhs &>()));
    STATIC_REQUIRE(noexcept(std::declval<const VoidLhs &>() == std::declval<const ObjectUnexpected &>()));

    // These are explicit overloads in C++17 and rewritten equality candidates in C++20 and later.
    STATIC_REQUIRE(noexcept(std::declval<const ObjectLhs &>() != std::declval<const ObjectRhs &>()));
    STATIC_REQUIRE(noexcept(std::declval<const NothrowConversionRhs &>() == std::declval<const ObjectLhs &>()));
    STATIC_REQUIRE(noexcept(std::declval<const ObjectLhs &>() != std::declval<const NothrowConversionRhs &>()));
    STATIC_REQUIRE(noexcept(std::declval<const NothrowConversionRhs &>() != std::declval<const ObjectLhs &>()));
    STATIC_REQUIRE(noexcept(std::declval<const ObjectUnexpected &>() == std::declval<const ObjectLhs &>()));
    STATIC_REQUIRE(noexcept(std::declval<const ObjectLhs &>() != std::declval<const ObjectUnexpected &>()));
    STATIC_REQUIRE(noexcept(std::declval<const ObjectUnexpected &>() != std::declval<const ObjectLhs &>()));
    STATIC_REQUIRE(noexcept(std::declval<const VoidLhs &>() != std::declval<const VoidRhs &>()));
    STATIC_REQUIRE(noexcept(std::declval<const ObjectUnexpected &>() == std::declval<const VoidLhs &>()));
    STATIC_REQUIRE(noexcept(std::declval<const VoidLhs &>() != std::declval<const ObjectUnexpected &>()));
    STATIC_REQUIRE(noexcept(std::declval<const ObjectUnexpected &>() != std::declval<const VoidLhs &>()));
}
