#include <utility>

#include <catch2/catch_test_macros.hpp>

#include <zeus/expected.hpp>

namespace
{

class ResourceOwningValue
{
public:
    ResourceOwningValue()
        : resource_(new char[1])
    {
        ++active_resources_;
    }

    ~ResourceOwningValue()
    {
        if (resource_ != nullptr)
        {
            delete[] resource_;
            --active_resources_;
        }
    }

    ResourceOwningValue(const ResourceOwningValue &)            = delete;
    ResourceOwningValue &operator=(const ResourceOwningValue &) = delete;
    ResourceOwningValue &operator=(ResourceOwningValue &&)      = delete;

    ResourceOwningValue(ResourceOwningValue &&other) noexcept
        : resource_(std::exchange(other.resource_, nullptr))
    {
    }

    static int active_resources() noexcept { return active_resources_; }

private:
    inline static int active_resources_ = 0;
    char             *resource_         = nullptr;
};

class ConvertibleValue
{
public:
    operator ResourceOwningValue() { return std::move(value_); }

private:
    ResourceOwningValue value_;
};

class NonDefaultConstructibleValue
{
public:
    NonDefaultConstructibleValue() = delete;
    NonDefaultConstructibleValue(int value)
        : value_(value)
    {
    }

    int value() const noexcept { return value_; }

private:
    int value_;
};

zeus::expected<ResourceOwningValue, int> make_resource_owning_value()
{
    zeus::expected<ConvertibleValue, int> source = ConvertibleValue {};
    return source;
}

} // namespace

TEST_CASE("converting expected does not leak value resources", "[expected][converting-constructor][lifetime]")
{
    // Adapted from the reproducer in https://github.com/TartanLlama/expected/issues/180.
    const int initial_active_resources = ResourceOwningValue::active_resources();

    {
        const auto result = make_resource_owning_value();
        REQUIRE(result.has_value());
    }

    REQUIRE(ResourceOwningValue::active_resources() == initial_active_resources);
}

TEST_CASE("converting expected does not require a default-constructible value", "[expected][converting-constructor]")
{
    const zeus::expected<int, int> source = 42;

    const zeus::expected<NonDefaultConstructibleValue, long> result = source;

    REQUIRE(result.has_value());
    REQUIRE(result->value() == 42);
}

TEST_CASE("converting an error does not construct the value alternative", "[expected][converting-constructor][lifetime]")
{
    const int initial_active_resources = ResourceOwningValue::active_resources();

    {
        zeus::expected<ConvertibleValue, int>           source(zeus::unexpect, 42);
        const zeus::expected<ResourceOwningValue, long> result(std::move(source));

        REQUIRE_FALSE(result.has_value());
        REQUIRE(result.error() == 42);
        REQUIRE(ResourceOwningValue::active_resources() == initial_active_resources);
    }

    REQUIRE(ResourceOwningValue::active_resources() == initial_active_resources);
}
