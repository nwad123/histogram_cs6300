#pragma once

#include "types.hpp"
#include <numeric>
#include <span>

namespace hpc::detail {
/// Calculates the index of the bin that the value should be placed in.
/// Precondition: Assumes ranges.size() == bins.size()
[[nodiscard]]
constexpr auto bin_index(const fp value, const std::span<fp> ranges) -> size_t
{
    return std::accumulate(ranges.begin(), ranges.end(), size_t{ 0 }, [&](auto &&index, const auto range) {
        if (value > range) { index += 1; }
        return index;
    });
}

// Update the counts and the maximum value of a bin, given the bin, ranges,
// and value
constexpr auto insert_to_bin(const fp value, const std::span<fp> ranges, Bin &bin) -> void
{
    const auto index = bin_index(value, ranges);

    // increment counts
    bin.counts[index]++;
    // update max
    const auto max = bin.maxes[index];
    bin.maxes[index] = std::max(max, value);
}
} // namespace hpc::detail
