#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <exception>
#include <queue>
#include <functional>

namespace threading {

thread_local size_t thread_id = 0;
thread_local size_t num_threads = 1;
static const size_t max_threads = std::max(1u, std::thread::hardware_concurrency());

/** Map a function f over nthreads */
template <typename Func> void thread_map(size_t nthreads, Func f) {
  if (nthreads == 0)
    nthreads = max_threads;

  if (nthreads == 1) {
    f();
    return;
  }

  auto threads = std::make_unique<std::thread[]>(nthreads);
  for (size_t i = 0; i < nthreads; ++i) {
    threads[i] = std::thread([&f, i, nthreads] {
      thread_id = i;
      num_threads = nthreads;
      f();
    });
  }
  for (size_t i = 0; i < nthreads; ++i) {
    threads[i].join();
  }
}

inline int64_t ceildiv(int64_t a, int64_t b) {
  return (a + b - 1) / b;
}

template <typename Func>
inline void parallel_for(int thread_count, int64_t begin, int64_t end, Func f) {
  const auto chunk_size = ceildiv(end - begin, thread_count);
  threading::thread_map(thread_count,
    [&]{
      const auto tid = int64_t(threading::thread_id);
      const auto first = begin + tid * chunk_size;
      const auto last = std::min(end, first + chunk_size);

      f(first, last);
    });
}

} // namespace threading
