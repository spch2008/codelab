/**
* @file   gcc_order.cpp
* @author sunpengcheng(spch2008n@foxmail.com)
* @brief
**/

#include <atomic>
#include <cassert>
#include <thread>
#include <cstdlib>
#include <iostream>
using namespace std;

#define barrier() __asm__ __volatile__ ("" : : : "memory")
//#define barrier() std::atomic_signal_fence(std::memory_order_relaxed)
//#define barrier() std::atomic_thread_fence(std::memory_order_release)

int i = 0;
int message[100];
int ready = 0;

int thread_1_loop = 0;
int thread_2_loop = 0;

void thread_1() {
  /* random delay */
  while (++thread_1_loop % 5 != 0) {}

  message[i/10] = 42;
  //barrier();
  ready = 1;
}

void thread_2() {
  /* random delay */
  while (++thread_2_loop % 5 != 0) {}

  if (ready == 1) {
    assert(message[5] == 42);
  }
}

int main() {
  while (true) {
    message[5] = 0;
    ready = 0;
    i = 56;

    thread_1_loop = rand();
    thread_2_loop = rand();

    thread a(thread_1);
    thread b(thread_2);
    a.join();
    b.join();
  }
  return 0;
}
