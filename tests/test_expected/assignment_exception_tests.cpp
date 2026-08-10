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

    int        payload() const noexcept { return payload_; }
    static int live_instances() noexcept { return live_instances_; }

private:
    inline static int live_instances_ = 0;
    int               payload_        = 0;
};

class ThrowOnCopy
{
public:
    explicit ThrowOnCopy(int payload = 0)
        : payload_(payload)
    {
    }

    ThrowOnCopy(const ThrowOnCopy &) { throw ConstructionException {}; }
    ThrowOnCopy(ThrowOnCopy &&) noexcept            = default;
    ThrowOnCopy &operator=(const ThrowOnCopy &)     = default;
    ThrowOnCopy &operator=(ThrowOnCopy &&) noexcept = default;

    int payload() const noexcept { return payload_; }

private:
    int payload_ = 0;
};

class ThrowOnMove
{
public:
    explicit ThrowOnMove(int payload = 0)
        : payload_(payload)
    {
    }

    ThrowOnMove(const ThrowOnMove &)            = delete;
    ThrowOnMove &operator=(const ThrowOnMove &) = delete;
    ThrowOnMove(ThrowOnMove &&) { throw ConstructionException {}; }
    ThrowOnMove &operator=(ThrowOnMove &&) = default;

    int payload() const noexcept { return payload_; }

private:
    int payload_ = 0;
};

struct ValueSource
{
};

class ThrowOnValueConversion
{
public:
    ThrowOnValueConversion(const ValueSource &) { throw ConstructionException {}; }
    ThrowOnValueConversion(ThrowOnValueConversion &&) noexcept(false) {}
    ThrowOnValueConversion &operator=(const ValueSource &) { return *this; }
};

struct ErrorSource
{
};

class ThrowOnErrorConversion
{
public:
    ThrowOnErrorConversion(const ErrorSource &) { throw ConstructionException {}; }
    ThrowOnErrorConversion(ThrowOnErrorConversion &&) noexcept(false) {}
    ThrowOnErrorConversion &operator=(const ErrorSource &) { return *this; }
};

} // namespace

TEST_CASE("copy assignment preserves an error when value construction throws", "[expected][assignment][exception-safety]")
{
    using Error    = NothrowMoveState<0>;
    using Expected = zeus::expected<ThrowOnCopy, Error>;

    STATIC_REQUIRE_FALSE(std::is_nothrow_copy_constructible_v<ThrowOnCopy>);
    STATIC_REQUIRE(std::is_nothrow_move_constructible_v<ThrowOnCopy>);

    const int initial_live_errors = Error::live_instances();

    {
        Expected       target(zeus::unexpect, 42);
        const Expected source(std::in_place, 7);

        REQUIRE_THROWS_AS(target = source, ConstructionException);
        REQUIRE_FALSE(target.has_value());
        REQUIRE(target.error().payload() == 42);
        REQUIRE(source.has_value());
        REQUIRE(source->payload() == 7);
        REQUIRE(Error::live_instances() == initial_live_errors + 1);
    }

    REQUIRE(Error::live_instances() == initial_live_errors);
}

TEST_CASE("copy assignment preserves a value when error construction throws", "[expected][assignment][exception-safety]")
{
    using Value    = NothrowMoveState<1>;
    using Expected = zeus::expected<Value, ThrowOnCopy>;

    STATIC_REQUIRE_FALSE(std::is_nothrow_copy_constructible_v<ThrowOnCopy>);
    STATIC_REQUIRE(std::is_nothrow_move_constructible_v<ThrowOnCopy>);

    const int initial_live_values = Value::live_instances();

    {
        Expected       target(std::in_place, 42);
        const Expected source(zeus::unexpect, 7);

        REQUIRE_THROWS_AS(target = source, ConstructionException);
        REQUIRE(target.has_value());
        REQUIRE(target->payload() == 42);
        REQUIRE_FALSE(source.has_value());
        REQUIRE(source.error().payload() == 7);
        REQUIRE(Value::live_instances() == initial_live_values + 1);
    }

    REQUIRE(Value::live_instances() == initial_live_values);
}

TEST_CASE("move assignment restores an error when value construction throws", "[expected][assignment][exception-safety]")
{
    using Error    = NothrowMoveState<2>;
    using Expected = zeus::expected<ThrowOnMove, Error>;

    STATIC_REQUIRE_FALSE(std::is_nothrow_move_constructible_v<ThrowOnMove>);
    STATIC_REQUIRE(std::is_nothrow_move_constructible_v<Error>);

    const int initial_live_errors = Error::live_instances();

    {
        Expected target(zeus::unexpect, 42);
        Expected source(std::in_place, 7);

        REQUIRE_THROWS_AS(target = std::move(source), ConstructionException);
        REQUIRE_FALSE(target.has_value());
        REQUIRE(target.error().payload() == 42);
        REQUIRE(source.has_value());
        REQUIRE(source->payload() == 7);
        REQUIRE(Error::live_instances() == initial_live_errors + 1);
    }

    REQUIRE(Error::live_instances() == initial_live_errors);
}

TEST_CASE("move assignment restores a value when error construction throws", "[expected][assignment][exception-safety]")
{
    using Value    = NothrowMoveState<3>;
    using Expected = zeus::expected<Value, ThrowOnMove>;

    STATIC_REQUIRE(std::is_nothrow_move_constructible_v<Value>);
    STATIC_REQUIRE_FALSE(std::is_nothrow_move_constructible_v<ThrowOnMove>);

    const int initial_live_values = Value::live_instances();

    {
        Expected target(std::in_place, 42);
        Expected source(zeus::unexpect, 7);

        REQUIRE_THROWS_AS(target = std::move(source), ConstructionException);
        REQUIRE(target.has_value());
        REQUIRE(target->payload() == 42);
        REQUIRE_FALSE(source.has_value());
        REQUIRE(source.error().payload() == 7);
        REQUIRE(Value::live_instances() == initial_live_values + 1);
    }

    REQUIRE(Value::live_instances() == initial_live_values);
}

TEST_CASE("value assignment restores an error when conversion throws", "[expected][assignment][exception-safety]")
{
    using Error    = NothrowMoveState<4>;
    using Expected = zeus::expected<ThrowOnValueConversion, Error>;

    STATIC_REQUIRE_FALSE(std::is_nothrow_constructible_v<ThrowOnValueConversion, const ValueSource &>);
    STATIC_REQUIRE_FALSE(std::is_nothrow_move_constructible_v<ThrowOnValueConversion>);
    STATIC_REQUIRE(std::is_nothrow_move_constructible_v<Error>);

    const int initial_live_errors = Error::live_instances();

    {
        Expected          target(zeus::unexpect, 42);
        const ValueSource source;

        REQUIRE_THROWS_AS(target = source, ConstructionException);
        REQUIRE_FALSE(target.has_value());
        REQUIRE(target.error().payload() == 42);
        REQUIRE(Error::live_instances() == initial_live_errors + 1);
    }

    REQUIRE(Error::live_instances() == initial_live_errors);
}

TEST_CASE("unexpected assignment restores a value when conversion throws", "[expected][assignment][exception-safety]")
{
    using Value    = NothrowMoveState<5>;
    using Expected = zeus::expected<Value, ThrowOnErrorConversion>;

    STATIC_REQUIRE(std::is_nothrow_move_constructible_v<Value>);
    STATIC_REQUIRE_FALSE(std::is_nothrow_constructible_v<ThrowOnErrorConversion, const ErrorSource &>);
    STATIC_REQUIRE_FALSE(std::is_nothrow_move_constructible_v<ThrowOnErrorConversion>);

    const int initial_live_values = Value::live_instances();

    {
        Expected                            target(std::in_place, 42);
        const zeus::unexpected<ErrorSource> source(ErrorSource {});

        REQUIRE_THROWS_AS(target = source, ConstructionException);
        REQUIRE(target.has_value());
        REQUIRE(target->payload() == 42);
        REQUIRE(Value::live_instances() == initial_live_values + 1);
    }

    REQUIRE(Value::live_instances() == initial_live_values);
}

TEST_CASE("copy assignment of a void expected retains a value when error construction throws", "[expected][assignment][exception-safety]")
{
    using Expected = zeus::expected<void, ThrowOnCopy>;

    Expected       target;
    const Expected source(zeus::unexpect, 7);

    REQUIRE_THROWS_AS(target = source, ConstructionException);
    REQUIRE(target.has_value());
    REQUIRE_FALSE(source.has_value());
    REQUIRE(source.error().payload() == 7);
}

TEST_CASE("move assignment of a void expected retains a value when error construction throws", "[expected][assignment][exception-safety]")
{
    using Expected = zeus::expected<void, ThrowOnMove>;

    Expected target;
    Expected source(zeus::unexpect, 7);

    REQUIRE_THROWS_AS(target = std::move(source), ConstructionException);
    REQUIRE(target.has_value());
    REQUIRE_FALSE(source.has_value());
    REQUIRE(source.error().payload() == 7);
}

TEST_CASE("unexpected assignment of a void expected retains a value when conversion throws", "[expected][assignment][exception-safety]")
{
    zeus::expected<void, ThrowOnErrorConversion> target;
    const zeus::unexpected<ErrorSource>          source(ErrorSource {});

    REQUIRE_THROWS_AS(target = source, ConstructionException);
    REQUIRE(target.has_value());
}
