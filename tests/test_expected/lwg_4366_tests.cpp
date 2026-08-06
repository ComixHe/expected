#include <catch2/catch_all.hpp>

#include <zeus/expected.hpp>

using namespace zeus;

namespace
{

// P3379R0 accepts comparison results that are implicitly convertible to bool.
// LWG 4366 makes the function bodies honor that constraint instead of requiring
// an explicit conversion.
struct ImplicitBoolean
{
    bool value;

    constexpr operator bool() const noexcept { return value; }
    explicit  operator bool() = delete;
};

struct Lhs
{
    explicit constexpr Lhs(int v)
        : value(v)
    {
    }

    int value;
};

struct Rhs
{
    explicit constexpr Rhs(int v)
        : value(v)
    {
    }

    int value;
};

ImplicitBoolean operator==(const Lhs &lhs, const Rhs &rhs)
{
    return {lhs.value == rhs.value};
}

} // namespace

TEST_CASE("LWG 4366 uses implicit conversion for expected-to-value comparison", "[LWG-4366][equality]")
{
    const expected<Lhs, int> value {Lhs {42}};

    CHECK(value == Rhs {42});
    CHECK_FALSE(value == Rhs {7});
}

TEST_CASE("LWG 4366 uses implicit conversion for expected-to-unexpected comparison", "[LWG-4366][equality]")
{
    const expected<int, Lhs> error {unexpect, 42};

    CHECK(error == zeus::unexpected<Rhs> {Rhs {42}});
    CHECK_FALSE(error == zeus::unexpected<Rhs> {Rhs {7}});
}

TEST_CASE("LWG 4366 uses implicit conversion for expected<void, E> comparison", "[LWG-4366][equality]")
{
    const expected<void, Lhs> lhs {unexpect, 42};

    CHECK(lhs == expected<void, Rhs> {unexpect, 42});
    CHECK_FALSE(lhs == expected<void, Rhs> {unexpect, 7});
}

TEST_CASE("LWG 4366 uses implicit conversion for expected<void, E>-to-unexpected comparison", "[LWG-4366][equality]")
{
    const expected<void, Lhs> error {unexpect, 42};

    CHECK(error == zeus::unexpected<Rhs> {Rhs {42}});
    CHECK_FALSE(error == zeus::unexpected<Rhs> {Rhs {7}});
}
