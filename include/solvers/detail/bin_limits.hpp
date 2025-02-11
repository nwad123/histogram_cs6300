#pragma once

#include "types.hpp"
#include <algorithm>

namespace hpc::detail {
/// This function calculates what the bounds are for each bin when given the number of
/// bins and the range of the dataset.
///
/// Precondition:
///     `range` must be a sorted pair, where `first <= second` always holds
///     `ranges` must be a iterable container with a size 
///     `ranges.size()` must be the number of bins desired
///
/// Postcondition:
///     `ranges` will be equal to [range.min + 1*step, range.min + 2*step ...]
static constexpr auto bin_upper_bounds(
    /*in*/ const std::pair<fp, fp> range,
    /*inout*/ auto &ranges
) -> void
{
    const auto max = range.second;
    const auto min = range.first;
    const auto num_bins = static_cast<fp>(ranges.size());

    fp init{ min };
    const fp diff{ (max - min) / num_bins };

    auto next = [&]() -> fp {
        init += diff;
        return init;
    };

    std::ranges::generate(ranges, next);
}

/// This function calculates what the bounds are for each bin when given the number of
/// bins and the range of the dataset and returns the bounds as a `std::vector`.
///
/// Precondition: `range` must be a sorted pair, where `first <= second` always holds
[[nodiscard]]
static constexpr auto bin_upper_bounds(
    /*in*/ const size_t num_bins,
    /*in*/ const std::pair<fp, fp> range
) -> std::vector<fp>
{
    std::vector<fp> ranges(num_bins);
    bin_upper_bounds(range, ranges);

    return ranges;
}
} // namespace hpc::detail
