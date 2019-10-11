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

class latch {
  std::atomic<size_t> num_left_;
  std::mutex mut_;
  std::condition_variable completed_;
  using lock_t = std::unique_lock<std::mutex>;

public:
  latch(size_t n) : num_left_(n) {}

  void count_down() {
    lock_t lock(mut_);
    if (--num_left_)
      return;
    completed_.notify_all();
  }

  void wait() {
    lock_t lock(mut_);
    completed_.wait(lock, [this] { return is_ready(); });
  }
  bool is_ready() { return num_left_ == 0; }
};

template <typename T> class concurrent_queue {
  std::queue<T> q_;
  std::mutex mut_;
  std::condition_variable item_added_;
  bool shutdown_;
  using lock_t = std::unique_lock<std::mutex>;

public:
  concurrent_queue() : shutdown_(false) {}

  void push(T val) {
    {
      lock_t lock(mut_);
      if (shutdown_)
        throw std::runtime_error("Item added to queue after shutdown");
      q_.push(move(val));
    }
    item_added_.notify_one();
  }

  bool pop(T &val) {
    lock_t lock(mut_);
    item_added_.wait(lock, [this] { return (!q_.empty() || shutdown_); });
    if (q_.empty())
      return false; // We are shutting down

    val = std::move(q_.front());
    q_.pop();
    return true;
  }

  void shutdown() {
    {
      lock_t lock(mut_);
      shutdown_ = true;
    }
    item_added_.notify_all();
  }

  void restart() { shutdown_ = false; }
};

class thread_pool {
  concurrent_queue<std::function<void()>> work_queue_;
  std::vector<std::thread> threads_;

  void worker_main() {
    std::function<void()> work;
    while (work_queue_.pop(work))
      work();
  }

  void create_threads() {
    size_t nthreads = threads_.size();
    for (size_t i = 0; i < nthreads; ++i) {
      try {
        threads_[i] = std::thread([this] { worker_main(); });
      } catch (...) {
        shutdown();
        throw;
      }
    }
  }

public:
  explicit thread_pool(size_t nthreads) : threads_(nthreads) {
    create_threads();
  }

  thread_pool() : thread_pool(max_threads) {}

  ~thread_pool() { shutdown(); }

  void submit(std::function<void()> work) { work_queue_.push(move(work)); }

  void shutdown() {
    work_queue_.shutdown();
    for (auto &thread : threads_)
      if (thread.joinable())
        thread.join();
  }

  void restart() {
    work_queue_.restart();
    create_threads();
  }
};

thread_pool & get_pool() {
  static thread_pool pool;
  return pool;
}

/** Map a function f over nthreads */
template <typename Func> void thread_map(size_t nthreads, Func f) {
  if (nthreads == 0)
    nthreads = max_threads;

  if (nthreads == 1) {
    f();
    return;
  }

  auto &pool = get_pool();
  latch counter(nthreads);
  for (size_t i = 0; i < nthreads; ++i) {
    pool.submit([&f, &counter, i, nthreads] {
      thread_id = i;
      num_threads = nthreads;
      f();
      counter.count_down();
    });
  }
  counter.wait();
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
