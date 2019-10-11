#include "kernels.h"
#include <cstdio>

int main(int argc, char ** argv) {
  if (argc != 2) {
    printf("Usage: %s thread_count\n", argv[0]);
    return EXIT_FAILURE;
  }
  auto thread_count = std::stoi(argv[1]);

  memory_bound(thread_count);
}
