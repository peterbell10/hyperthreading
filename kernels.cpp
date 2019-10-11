#include "kernels.h"
#include "thread_pool.h"
#include "iaca/iacaMarks.h"

#include <mutex>
#include <memory>
#include <cmath>


double add_bound(int thread_count) {
  double total = 0.0;
  std::mutex mut_;
  constexpr int64_t N = 7'000'000'000;


  threading::parallel_for(thread_count, 1, N,
    [&](int64_t begin, int64_t end){
      double sum = 0.;
      for (double i = begin; i < end; i += 1.) {
        // IACA_START
        sum += i * 1.e-30;
      }
      // IACA_END

      std::lock_guard<std::mutex> lock(mut_);
      total += sum;
    });

  return total;
}

double div_bound(int thread_count) {
  double total = 0.0;
  std::mutex mut_;
  constexpr int64_t N = 2'489'500'000;

  threading::parallel_for(thread_count, 1, N,
    [&](int64_t begin, int64_t end){
      double sum = 0.;

      for (int64_t i = begin; i < end; ++i) {
        // IACA_START
        sum += 1. / i;
      }
      // IACA_END

      std::lock_guard<std::mutex> lock(mut_);
      total += sum;
    });

  return total;
}

void memory_bound(int thread_count) {
  constexpr int64_t l1_cache = 32 * 1024;
  constexpr int64_t N = 22'150'000;
  constexpr int64_t data_size = l1_cache / sizeof(int64_t) / 2;
  auto d1 = std::make_unique<int64_t[]>(data_size);
  auto d2 = std::make_unique<int64_t[]>(data_size);


  threading::parallel_for(thread_count, 0, data_size,
    [&] (int64_t begin, int64_t end) {
      auto input = d1.get(), output = d2.get();
      for (int64_t j = begin; j < end; ++j) {
        input[j] = j;
      }
      asm volatile ("" :: "r" (input), "r" (output));

      for (int i = 0; i < N; ++i) {
        for (int64_t j = begin; j < end; ++j) {
          // IACA_START
          output[j] = input[j];
        }
        // IACA_END
        asm volatile ("" ::: "memory");
      }
    });
}

constexpr int Nbody = 512, Ndim = 3;
constexpr double G = 6.674e-11;
double pos[Nbody][Ndim]{};
double mass[Nbody]{};
double f[Nbody][Ndim]{};

void nbody_forces(int thread_count)
{
  constexpr int repeats = 8175;
  threading::parallel_for(thread_count, 0, Nbody,
    [&] (int64_t begin, int64_t end) {
      for (int i = 0; i < repeats; ++i) {
        for (int64_t i = begin; i < end; i++) {
          for (int64_t j = 0; j < Nbody; j++) {
            IACA_START
            double r = 0.0;
            for (int l = 0; l < Ndim; ++l) {
              double dr = pos[i][l] - pos[j][l];
              r += dr * dr;
            }
            r = std::sqrt(r);

            const double weight = G * mass[i] * mass[j] / (r * r * r);
            for (int l = 0; l < Ndim; l++) {
              f[i][l] += weight * (pos[j][l] - pos[i][l]);
            }
          }
          IACA_END
        }
      }
    });
}
