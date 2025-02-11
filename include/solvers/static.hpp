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
};

// The `StaticTree` class must satisfy the `Solver` concept
constexpr Config config = {};
static_assert(Solver<StaticTree<config>>);

} // namespace hpc
