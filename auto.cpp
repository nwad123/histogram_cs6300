#include "dataset_generator.hpp"
#include "solver_timer.hpp"
#include "solvers/parallel.hpp"
#include "solvers/serial.hpp"
#include "solvers/tree.hpp"
#include "types.hpp"

#include <initializer_list>
#include <random>

using namespace hpc;

// constants
static constexpr auto threads = std::initializer_list<size_t>{ 1, 2, 4, 8, 16, 32, 64, 128 };
static constexpr auto sizes =
    std::initializer_list<size_t>{ 10'000, 100'000, 1'000'000, 10'000'000, 100'000'000, 1'000'000'000, 10'000'000'000 };

static constexpr fp min = 0.0;
static constexpr fp max = 5.0;
static constexpr auto bins = 5;

static constexpr size_t reps = 10;

// Runs timing analysis on all combinations of `threads` and `sizes`. `reps` indicates how many 
// iterations of the test should be run. More iterations give better data (especially for std 
// deviation) but take linearly longer.
auto main() -> int
{
    // store all of our output data in the results array
    std::vector<Result> results;

    // start the csv header
    fmt::println("Size,Name,Threads,Mean,Stddev,Min,Max");

    // handrolled cartesian product over all iterations
    for (const auto size : sizes) {
        auto dataset = hpc::make_dataset(min, max, size, std::mt19937{ 100 });

        // perform serial timing results
        {
            const auto serial_config = Config(1, bins, min, max, size);
            for (const auto &result : SolverTimer<reps>(serial_config, dataset)(Serial{})) {
                results.emplace_back(result); // push results into the results array
            }
        }

        // perform parallel timing across a variety of thread counts
        for (const auto num_threads : threads) {
            const auto config = Config(num_threads, bins, min, max, size);

            for (const auto &result : SolverTimer<reps>{ config, dataset }(Parallel{}, Tree{})) {
                results.emplace_back(result);
            }
        }

        // Output the data for this specific size
        for (const auto &result : results) {
            fmt::println(
                "{},{},{},{},{},{},{}",
                size,
                result.name,
                result.config.threads,
                result.time_ms,
                result.std_dev_ms,
                result.min_ms,
                result.max_ms
            );
        }
        results.clear();
    }
}
