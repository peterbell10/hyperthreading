#pragma once
#include <thread>


double div_bound(int thread_count);
double add_bound(int thread_count);
void memory_bound(int thread_count);
void nbody_forces(int thread_count);

struct jthread: public std::thread {
  using std::thread::thread;
  ~jthread() { std::thread::join(); }
};
