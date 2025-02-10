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

    auto insert = [&ranges, &bin](/*in*/ const fp value) {
        auto bin_it = std::upper_bound(ranges.cbegin(), ranges.cend(), value);
        // since bin_it will be always be equal to or ahead of `ranges.begin()`
        // we can assume that this will always be positive, and therefore no
        // problem to cast it to a size_t
        auto index = static_cast<size_t>(std::distance(ranges.cbegin(), bin_it));

        bin.counts[index]++;
        if (bin.maxes[index] < value) { bin.maxes[index] = value; }
    };

    // Calculate the bins for the whole dataset
    for (const auto data : dataset) { hpc::detail::insert_to_bin(data, ranges, bin); }

    return bin;
}
}// namespace hpc
