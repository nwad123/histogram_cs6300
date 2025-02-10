#include "solvers/serial.hpp"
#include "solvers/detail/bin_index.hpp"

#include <algorithm>

namespace hpc {
[[nodiscard]] auto Serial::operator()(const Config &config, const std::span<fp> dataset) const -> Bin
{
    Bin bin;
    bin.maxes.resize(config.bins);
    bin.counts.resize(config.bins);

    std::vector<fp> ranges(config.bins);
    fp f{ config.max };
    fp diff{ (config.max - config.min) / static_cast<fp>(config.bins) };

    for (auto it = ranges.rbegin(); it != ranges.rend(); ++it) {
        auto &range = *it;
        range = f;
        f -= diff;
    }

    // Calculate the bins for the whole dataset
    for (const auto data : dataset) { hpc::detail::insert_to_bin(data, ranges, bin); }

    return bin;
}
}// namespace hpc
