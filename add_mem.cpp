#include "kernels.h"
#include <cstdio>

int main(int argc, char ** argv) {
  if (argc != 2) {
    printf("Usage: %s thread_count\n", argv[0]);
    return EXIT_FAILURE;
  }
  auto thread_count = std::stoi(argv[1]);

  jthread t1(add_bound, thread_count);
  jthread t2(memory_bound, thread_count);
}
