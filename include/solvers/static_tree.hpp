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
    return {};
}

} // namespace hpc
