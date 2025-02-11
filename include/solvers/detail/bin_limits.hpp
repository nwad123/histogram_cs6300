#pragma once

#include "types.hpp"
#include <algorithm>

namespace hpc::detail {

/// This function calculates what the bounds are for each bin when given the number of
/// bins and the range of the dataset.
///
/// Precondition: `range` must be a sorted pair, where `first <= second` always holds
[[nodiscard]]
static constexpr auto get_bin_steps(
    /*in*/ const size_t num_bins,
    /*in*/ const std::pair<fp, fp> range
) -> std::vector<fp>
{
    const auto max = range.second;
    const auto min = range.first;

    std::vector<fp> ranges(num_bins);

    fp init{ min };
    const fp diff{ (max - min) / static_cast<fp>(num_bins) };

    auto next = [&]() -> fp {
        init += diff;
        return init;
    };

    std::ranges::generate(ranges, next);

    return ranges;
}
} // namespace hpc::detail
