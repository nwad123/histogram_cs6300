#include "solvers/parallel.hpp"
#include "solvers/detail/bin_index.hpp"
#include "solvers/detail/bin_limits.hpp"

#include <algorithm>
#include <thread>

namespace hpc {
[[nodiscard]] auto Parallel::operator()(const Config &config, const std::span<fp> dataset) const -> Bin
{
    using std::thread;
    using std::ref;

    // This is the function that each thread performs. The basic steps are as follows:
    // 1. Calculate what subset of data to operate on.
    // 2. Calculate the bins for the subset of data.
    auto task = [&config, &dataset](/*in*/ size_t id, /*in*/ const std::span<fp> ranges, /*out*/ Bin &bin) {
        const auto num_elements = config.size / config.threads;
        const auto starting_element = id * num_elements;

        const auto dataset_slice = [&]() {
            // in the case that we cannot evenly divide our data elements between threads
            // we need to ensure that the last thread picks up the extra elements that were
            // not allocated a thread
            const auto max_thread_id = config.threads - 1;
            if (id < max_thread_id) {
                return dataset.subspan(starting_element, num_elements);
            } else {
                return dataset.subspan(starting_element);
            }
        }();

        bin.maxes.resize(config.bins);
        bin.counts.resize(config.bins);

        for (const auto data : dataset_slice) { hpc::detail::insert_to_bin(data, ranges, bin); }
    };

    auto ranges = detail::get_bin_steps(config.bins, { config.min, config.max });
    auto bins = std::vector<Bin>{ config.threads };

    std::vector<thread> threads{};
    threads.reserve(config.threads);

    // Calculate the bins on each thread separately
    for (size_t id = 0; id < config.threads; id++) {
        threads.push_back(thread{ task, id, std::span{ ranges }, ref(bins[id]) });
    }

    // Wait for each thread to finish
    for (auto &thread : threads) { thread.join(); }

    // Merge the output from all threads together
    Bin output{};
    output.maxes.resize(config.bins);
    output.counts.resize(config.bins);

    for (auto &bin : bins) {
        for (size_t i = 0; i < config.bins; i++) {
            output.maxes[i] = std::max(bin.maxes[i], output.maxes[i]);
            output.counts[i] += bin.counts[i];
        }
    }

    return output;
}
} // namespace hpc
