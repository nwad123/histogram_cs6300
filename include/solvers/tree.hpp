#pragma once

#include "concepts.hpp"
#include "types.hpp"

#include <semaphore>

namespace hpc {
/// The `Tree` solver implements a multithreaded Tree Structured Sum approach to generating a
/// histogram.
class Tree
{
  public:
    static constexpr std::string_view name = "TreeStructuredSum";

    /// `Tree.operator()` generates the histogram bins based of the config and input dataset.
    /// Note that the data under the `dataset` span must outlive the `Tree` object as no
    /// attempt to copy or take ownership of the data is made.
    [[nodiscard]]
    auto operator()(const Config &config, const std::span<fp> dataset) const -> Bin;

  protected:
    /// The `Tree` class has a `detail` subclass that abstracts away some parts of the histogram
    /// algorithm. This is to provide more readable code, and as often as possible functions are
    /// marked inline, constexpr, or both in order to assure that high performance can still
    /// be achieved.
    struct detail
    {
        /// The Tree Structured sum approach works by merging the work done by the different
        /// threads together in a tree-like manner across threads. For example, if you had
        /// four threads then thread 1 would be in charge of merging the work of thread 2 and
        /// thread 3 together, and thread 3 would be in charge of merging the work of thread
        /// 4. See the example below:
        /*
         * Threads: 1  2  3  4
         *          | /   | /
         *          +     +
         *          |    /
         *          |  /
         *          +
         *          <Final result in thread 1>
         */
        /// Each thread needs to know what threads it can receive from, and it also need to know
        /// when it's done being used. This function provides a static way of determining what
        /// threads a thread can recieve from.
        ///
        /// `num_threads` is the total number of threads. Must be > 0
        /// `thread_id` is the id of the thread that the recieve list is to be generated for.
        ///     0 < `thread_id` < `num_threads`
        [[nodiscard]]
        static constexpr auto get_receive_list(
            /*in*/ const size_t num_threads,
            /*in*/ const size_t thread_id
        ) -> std::vector<size_t>;

        /// Calculates the number of receives that thread `i` will perform statically.
        /// Requires that `i < num_threads`
        [[nodiscard]]
        static constexpr auto num_receives(/*in*/ const size_t num_threads, /*in*/ const size_t thread_id) -> size_t;

        /// C++ semaphores can't be default constructed, so I wrote a wrapper class to default
        /// construct them.
        struct simple_semaphore
        {
            std::binary_semaphore sem_;

            simple_semaphore() : sem_(0) {}
            auto acquire() -> void { sem_.acquire(); }
            auto release() -> void { sem_.release(); }
        };

        // For each thread we'll need a "send" and "recieve" semaphore to synchronize the passing
        // of bins between threads. Prior to this struct I implemented this as a pair of vectors,
        // one for sending and one for receiving. However I noticed that when a reciever semaphore
        // is accessed the sender semaphore is soon accessed and vice versa, so it makes sense to
        // them local to each other in memory.
        struct semaphore_pair
        {
            simple_semaphore sender;
            simple_semaphore receiver;
        };

        /// `ThreadSemaphores` provides a simple abstraction for the actions that each thread can
        /// take in the `Tree` class. The names of the methods are self-explanatory.
        struct ThreadSemaphores
        {
            std::vector<semaphore_pair> semaphores_;

            explicit ThreadSemaphores(/*in*/ const size_t num_threads)
                : semaphores_(std::vector<semaphore_pair>(num_threads))
            {}

            /// Indicates that the caller thread is ready to receive bins from thread `id`
            inline auto ready_to_recv_from(/*in*/ const size_t id) -> void { semaphores_[id].sender.acquire(); }

            /// Indicates that the caller thread is done receiving bins from thread `id`, which
            /// indicates to thread `id` that they can close.
            inline auto done_recving_from(/*in*/ const size_t id) -> void { semaphores_[id].receiver.release(); }

            /// Indicates that all work has be completed on thread `id` and that it's ready
            /// to be consumed.
            inline auto completed_work_on(/*in*/ const size_t id) -> void
            {
                semaphores_[id].sender.release();
                semaphores_[id].receiver.acquire();
            }
        };
    }; // struct detail
};

// `Tree` must satisfy the `Solver` concept
static_assert(Solver<Tree>);

} // namespace hpc

[[nodiscard]]
constexpr auto hpc::Tree::detail::num_receives(const size_t num_threads, const size_t thread_id) -> size_t
{
    auto bit_log2 = [](const auto of) {
        const auto bit_width = std::bit_width(of);
        return static_cast<size_t>(bit_width) - size_t{ 1 };
    };

    if (thread_id == 0) { return bit_log2(std::bit_ceil(num_threads)); }
    if (thread_id % 2 == 1) { return 0; }
    if (std::has_single_bit(thread_id)) { return bit_log2(thread_id); }

    const auto i_ = thread_id - std::bit_floor(thread_id);

    return num_receives(num_threads, i_);
};

[[nodiscard]]
constexpr auto hpc::Tree::detail::get_receive_list(
    /*in*/ const size_t num_threads,
    /*in*/ const size_t thread_id
) -> std::vector<size_t>
{
    std::vector<size_t> recieves(num_receives(num_threads, thread_id), size_t{ 0 });

    for (size_t i = 1; auto &recieve : recieves) {
        recieve = i + thread_id;
        i = i << 1;
    }

    return recieves;
}
