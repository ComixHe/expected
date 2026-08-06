#include <utility>

#include <catch2/catch_test_macros.hpp>

#include <zeus/expected.hpp>

namespace
{

struct ConstructorException
{
};

struct SourceValue
{
};

struct SourceError
{
};

template<int Id>
class ErrorDestructorProbe
{
public:
    ErrorDestructorProbe() = default;
    ErrorDestructorProbe(const SourceError &) {}
    ErrorDestructorProbe(SourceError &&) {}
    ErrorDestructorProbe(const ErrorDestructorProbe &)            = default;
    ErrorDestructorProbe(ErrorDestructorProbe &&)                 = default;
    ErrorDestructorProbe &operator=(const ErrorDestructorProbe &) = default;
    ErrorDestructorProbe &operator=(ErrorDestructorProbe &&)      = default;

    ~ErrorDestructorProbe() { ++destructor_calls_; }

    static int  destructor_calls() noexcept { return destructor_calls_; }
    static void reset() noexcept { destructor_calls_ = 0; }

private:
    inline static int destructor_calls_ = 0;
};

class ImplicitThrowFromSourceValue
{
public:
    ImplicitThrowFromSourceValue(const SourceValue &) { throw ConstructorException {}; }
    ImplicitThrowFromSourceValue(SourceValue &&) { throw ConstructorException {}; }
};

class ImplicitTargetValue
{
public:
    ImplicitTargetValue(const SourceValue &) {}
    ImplicitTargetValue(SourceValue &&) {}
};

template<int Id>
class ExplicitThrowFromSourceError
{
public:
    explicit ExplicitThrowFromSourceError(const SourceError &) { throw ConstructorException {}; }
    explicit ExplicitThrowFromSourceError(SourceError &&) { throw ConstructorException {}; }

    ~ExplicitThrowFromSourceError() { ++destructor_calls_; }

    static int  destructor_calls() noexcept { return destructor_calls_; }
    static void reset() noexcept { destructor_calls_ = 0; }

private:
    inline static int destructor_calls_ = 0;
};

class ThrowOnCopyValue
{
public:
    ThrowOnCopyValue() = default;
    ThrowOnCopyValue(const ThrowOnCopyValue &) { throw ConstructorException {}; }
    ThrowOnCopyValue(ThrowOnCopyValue &&)                 = default;
    ThrowOnCopyValue &operator=(const ThrowOnCopyValue &) = default;
    ThrowOnCopyValue &operator=(ThrowOnCopyValue &&)      = default;
};

class ThrowOnMoveValue
{
public:
    ThrowOnMoveValue()                         = default;
    ThrowOnMoveValue(const ThrowOnMoveValue &) = delete;
    ThrowOnMoveValue(ThrowOnMoveValue &&) { throw ConstructorException {}; }
    ThrowOnMoveValue &operator=(const ThrowOnMoveValue &) = delete;
    ThrowOnMoveValue &operator=(ThrowOnMoveValue &&)      = default;
};

template<int Id>
class ThrowOnCopyError
{
public:
    ThrowOnCopyError() = default;
    ThrowOnCopyError(const ThrowOnCopyError &) { throw ConstructorException {}; }
    ThrowOnCopyError(ThrowOnCopyError &&)                 = default;
    ThrowOnCopyError &operator=(const ThrowOnCopyError &) = default;
    ThrowOnCopyError &operator=(ThrowOnCopyError &&)      = default;

    ~ThrowOnCopyError() { ++destructor_calls_; }

    static int  destructor_calls() noexcept { return destructor_calls_; }
    static void reset() noexcept { destructor_calls_ = 0; }

private:
    inline static int destructor_calls_ = 0;
};

template<int Id>
class ThrowOnMoveError
{
public:
    ThrowOnMoveError()                         = default;
    ThrowOnMoveError(const ThrowOnMoveError &) = delete;
    ThrowOnMoveError(ThrowOnMoveError &&) { throw ConstructorException {}; }
    ThrowOnMoveError &operator=(const ThrowOnMoveError &) = delete;
    ThrowOnMoveError &operator=(ThrowOnMoveError &&)      = default;

    ~ThrowOnMoveError() { ++destructor_calls_; }

    static int  destructor_calls() noexcept { return destructor_calls_; }
    static void reset() noexcept { destructor_calls_ = 0; }

private:
    inline static int destructor_calls_ = 0;
};

template<class ErrorProbe, class Operation>
void require_no_destructor_for_unconstructed_error(Operation &&operation)
{
    ErrorProbe::reset();
    REQUIRE_THROWS_AS(std::forward<Operation>(operation)(), ConstructorException);
    REQUIRE(ErrorProbe::destructor_calls() == 0);
}

} // namespace

TEST_CASE(
    "implicit lvalue converting constructor does not destroy an unconstructed error", "[expected][constructor][exception-safety][lifetime]"
)
{
    using Source = zeus::expected<SourceValue, SourceError>;
    using Probe  = ErrorDestructorProbe<1>;
    using Target = zeus::expected<ImplicitThrowFromSourceValue, Probe>;

    const Source source(std::in_place);
    require_no_destructor_for_unconstructed_error<Probe>(
        [&]
        {
            Target target = source;
            (void) target;
        }
    );
}

TEST_CASE(
    "explicit lvalue converting constructor does not destroy an unconstructed error", "[expected][constructor][exception-safety][lifetime]"
)
{
    using Source = zeus::expected<SourceValue, SourceError>;
    using Probe  = ExplicitThrowFromSourceError<2>;
    using Target = zeus::expected<ImplicitTargetValue, Probe>;

    const Source source(zeus::unexpect);
    require_no_destructor_for_unconstructed_error<Probe>(
        [&]
        {
            Target target(source);
            (void) target;
        }
    );
}

TEST_CASE(
    "implicit rvalue converting constructor does not destroy an unconstructed error", "[expected][constructor][exception-safety][lifetime]"
)
{
    using Source = zeus::expected<SourceValue, SourceError>;
    using Probe  = ErrorDestructorProbe<3>;
    using Target = zeus::expected<ImplicitThrowFromSourceValue, Probe>;

    Source source(std::in_place);
    require_no_destructor_for_unconstructed_error<Probe>(
        [&]
        {
            Target target = std::move(source);
            (void) target;
        }
    );
}

TEST_CASE(
    "explicit rvalue converting constructor does not destroy an unconstructed error", "[expected][constructor][exception-safety][lifetime]"
)
{
    using Source = zeus::expected<SourceValue, SourceError>;
    using Probe  = ExplicitThrowFromSourceError<4>;
    using Target = zeus::expected<ImplicitTargetValue, Probe>;

    Source source(zeus::unexpect);
    require_no_destructor_for_unconstructed_error<Probe>(
        [&]
        {
            Target target(std::move(source));
            (void) target;
        }
    );
}

TEST_CASE("copying a value does not destroy an unconstructed error", "[expected][constructor][exception-safety][lifetime]")
{
    using Probe    = ErrorDestructorProbe<5>;
    using Expected = zeus::expected<ThrowOnCopyValue, Probe>;

    const Expected source(std::in_place);
    require_no_destructor_for_unconstructed_error<Probe>(
        [&]
        {
            Expected target(source);
            (void) target;
        }
    );
}

TEST_CASE("copying an error does not destroy an unconstructed error", "[expected][constructor][exception-safety][lifetime]")
{
    using Probe    = ThrowOnCopyError<6>;
    using Expected = zeus::expected<int, Probe>;

    const Expected source(zeus::unexpect);
    require_no_destructor_for_unconstructed_error<Probe>(
        [&]
        {
            Expected target(source);
            (void) target;
        }
    );
}

TEST_CASE("moving a value does not destroy an unconstructed error", "[expected][constructor][exception-safety][lifetime]")
{
    using Probe    = ErrorDestructorProbe<7>;
    using Expected = zeus::expected<ThrowOnMoveValue, Probe>;

    Expected source(std::in_place);
    require_no_destructor_for_unconstructed_error<Probe>(
        [&]
        {
            Expected target(std::move(source));
            (void) target;
        }
    );
}

TEST_CASE("moving an error does not destroy an unconstructed error", "[expected][constructor][exception-safety][lifetime]")
{
    using Probe    = ThrowOnMoveError<8>;
    using Expected = zeus::expected<int, Probe>;

    Expected source(zeus::unexpect);
    require_no_destructor_for_unconstructed_error<Probe>(
        [&]
        {
            Expected target(std::move(source));
            (void) target;
        }
    );
}

TEST_CASE("copying a void expected does not destroy an unconstructed error", "[expected][constructor][exception-safety][lifetime]")
{
    using Probe    = ThrowOnCopyError<9>;
    using Expected = zeus::expected<void, Probe>;

    const Expected source(zeus::unexpect);
    require_no_destructor_for_unconstructed_error<Probe>(
        [&]
        {
            Expected target(source);
            (void) target;
        }
    );
}

TEST_CASE("moving a void expected does not destroy an unconstructed error", "[expected][constructor][exception-safety][lifetime]")
{
    using Probe    = ThrowOnMoveError<10>;
    using Expected = zeus::expected<void, Probe>;

    Expected source(zeus::unexpect);
    require_no_destructor_for_unconstructed_error<Probe>(
        [&]
        {
            Expected target(std::move(source));
            (void) target;
        }
    );
}
