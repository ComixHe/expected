#include <type_traits>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include <zeus/expected.hpp>

namespace
{

struct ConstructionException
{
};

template<int Id>
class NothrowMoveState
{
public:
    explicit NothrowMoveState(int payload = 0)
        : payload_(payload)
    {
        ++live_instances_;
    }

    NothrowMoveState(const NothrowMoveState &other) noexcept
        : payload_(other.payload_)
    {
        ++live_instances_;
    }

    NothrowMoveState(NothrowMoveState &&other) noexcept
        : payload_(std::exchange(other.payload_, -1))
    {
        ++live_instances_;
    }

    NothrowMoveState &operator=(const NothrowMoveState &other) noexcept
    {
        payload_ = other.payload_;
        return *this;
    }

    NothrowMoveState &operator=(NothrowMoveState &&other) noexcept
    {
        payload_ = std::exchange(other.payload_, -1);
        return *this;
    }

    ~NothrowMoveState() { --live_instances_; }

    friend void swap(NothrowMoveState &lhs, NothrowMoveState &rhs) noexcept
    {
        using std::swap;
        swap(lhs.payload_, rhs.payload_);
    }

    int        payload() const noexcept { return payload_; }
    static int live_instances() noexcept { return live_instances_; }

private:
    inline static int live_instances_ = 0;
    int               payload_        = 0;
};

template<int Id>
class ThrowOnMoveState
{
public:
    explicit ThrowOnMoveState(int payload = 0)
        : payload_(payload)
    {
        ++live_instances_;
    }

    ThrowOnMoveState(const ThrowOnMoveState &)            = delete;
    ThrowOnMoveState &operator=(const ThrowOnMoveState &) = delete;

    ThrowOnMoveState(ThrowOnMoveState &&other)
        : payload_(other.payload_)
    {
        throw ConstructionException {};
    }

    ThrowOnMoveState &operator=(ThrowOnMoveState &&other) noexcept
    {
        payload_ = std::exchange(other.payload_, -1);
        return *this;
    }

    ~ThrowOnMoveState() { --live_instances_; }

    friend void swap(ThrowOnMoveState &lhs, ThrowOnMoveState &rhs) noexcept
    {
        using std::swap;
        swap(lhs.payload_, rhs.payload_);
    }

    int        payload() const noexcept { return payload_; }
    static int live_instances() noexcept { return live_instances_; }

private:
    inline static int live_instances_ = 0;
    int               payload_        = 0;
};

} // namespace

TEST_CASE("swap restores an error when value construction throws", "[expected][swap][exception-safety]")
{
    using Value    = ThrowOnMoveState<0>;
    using Error    = NothrowMoveState<0>;
    using Expected = zeus::expected<Value, Error>;

    STATIC_REQUIRE_FALSE(std::is_nothrow_move_constructible_v<Value>);
    STATIC_REQUIRE(std::is_nothrow_move_constructible_v<Error>);

    const int initial_live_values = Value::live_instances();
    const int initial_live_errors = Error::live_instances();

    {
        Expected value(std::in_place, 17);
        Expected error(zeus::unexpect, 42);

        REQUIRE_THROWS_AS(value.swap(error), ConstructionException);
        REQUIRE(value.has_value());
        REQUIRE(value->payload() == 17);
        REQUIRE_FALSE(error.has_value());
        REQUIRE(error.error().payload() == 42);
        REQUIRE(Value::live_instances() == initial_live_values + 1);
        REQUIRE(Error::live_instances() == initial_live_errors + 1);
    }

    REQUIRE(Value::live_instances() == initial_live_values);
    REQUIRE(Error::live_instances() == initial_live_errors);
}

TEST_CASE("swap restores a value when error construction throws", "[expected][swap][exception-safety]")
{
    using Value    = NothrowMoveState<1>;
    using Error    = ThrowOnMoveState<1>;
    using Expected = zeus::expected<Value, Error>;

    STATIC_REQUIRE(std::is_nothrow_move_constructible_v<Value>);
    STATIC_REQUIRE_FALSE(std::is_nothrow_move_constructible_v<Error>);

    const int initial_live_values = Value::live_instances();
    const int initial_live_errors = Error::live_instances();

    {
        Expected value(std::in_place, 17);
        Expected error(zeus::unexpect, 42);

        REQUIRE_THROWS_AS(value.swap(error), ConstructionException);
        REQUIRE(value.has_value());
        REQUIRE(value->payload() == 17);
        REQUIRE_FALSE(error.has_value());
        REQUIRE(error.error().payload() == 42);
        REQUIRE(Value::live_instances() == initial_live_values + 1);
        REQUIRE(Error::live_instances() == initial_live_errors + 1);
    }

    REQUIRE(Value::live_instances() == initial_live_values);
    REQUIRE(Error::live_instances() == initial_live_errors);
}

TEST_CASE("void swap retains states when moving an error into the left operand throws", "[expected][swap][exception-safety]")
{
    using Error    = ThrowOnMoveState<2>;
    using Expected = zeus::expected<void, Error>;

    const int initial_live_errors = Error::live_instances();

    {
        Expected value;
        Expected error(zeus::unexpect, 42);

        REQUIRE_THROWS_AS(value.swap(error), ConstructionException);
        REQUIRE(value.has_value());
        REQUIRE_FALSE(error.has_value());
        REQUIRE(error.error().payload() == 42);
        REQUIRE(Error::live_instances() == initial_live_errors + 1);
    }

    REQUIRE(Error::live_instances() == initial_live_errors);
}

TEST_CASE("void swap retains states when moving an error into the right operand throws", "[expected][swap][exception-safety]")
{
    using Error    = ThrowOnMoveState<3>;
    using Expected = zeus::expected<void, Error>;

    const int initial_live_errors = Error::live_instances();

    {
        Expected error(zeus::unexpect, 42);
        Expected value;

        REQUIRE_THROWS_AS(error.swap(value), ConstructionException);
        REQUIRE_FALSE(error.has_value());
        REQUIRE(error.error().payload() == 42);
        REQUIRE(value.has_value());
        REQUIRE(Error::live_instances() == initial_live_errors + 1);
    }

    REQUIRE(Error::live_instances() == initial_live_errors);
}
