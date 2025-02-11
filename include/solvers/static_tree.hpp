#pragma once

#include "concepts.hpp"
#include "types.hpp"

namespace hpc {
/// The `StaticTree` solver...
template<Config config>
class StaticTree
{
  public:
    static constexpr std::string_view name = "StaticTree";
    /// `StaticTree.operator()` ...
    [[nodiscard]] auto operator()([[maybe_unused]] const Config &_, const std::span<fp> dataset) const -> Bin;

    struct impl
    {
        using BinRanges = std::array<fp, config.bins>;
    };
};

// The `StaticTree` class must satisfy the `Solver` concept
constexpr Config config = {};
static_assert(Solver<StaticTree<config>>);

template<Config config>
[[nodiscard]]
auto StaticTree<config>::operator()([[maybe_unused]] const Config &_, const std::span<fp> dataset) const -> Bin
{
    // Algin to a cache line 
    constexpr auto cache_line_size = 128;
    alignas(cache_line_size) typename impl::BinRanges ranges{};

    // Get the bounds of the bin
    bin_upper_bounds({config.min, config.max}, ranges);


    return {};
}

} // namespace hpc
