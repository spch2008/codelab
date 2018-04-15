#include <atomic>
#include <thread>
#include <iostream>
#include <sstream>

struct SpinLock {
  static const size_t max_locks = 5;
  std::atomic<int> locks[max_locks];

  bool Lock(size_t const thread_id) {

    locks[thread_id].store(1, std::memory_order_release);    // Store
    //locks[thread_id].store(1, std::memory_order_seq_cst);    // Store

    for (size_t i = 0; i < max_locks; ++i)
      if (locks[i].load(std::memory_order_acquire) > 0 && i != thread_id) { // Load
          locks[thread_id].store(0, std::memory_order_release);   // undo lock
        return false;
      }
    return true;
  }

  void Unlock(size_t const thread_id) {
    locks[thread_id].store(0, std::memory_order_release);
  }
};

SpinLock spin_lock;
int counter = 0;

void Accumulate(size_t const thread_id)  {
  for (size_t i = 0; i < 100000; ++i) {
    while (!spin_lock.Lock(thread_id));
    ++counter;
    spin_lock.Unlock(thread_id);
  }
}

int main() {
  counter = 0;
  std::thread t0([&]() { Accumulate(0); });
  std::thread t1([&]() { Accumulate(1); });
  t0.join(); t1.join();

  std::cout << counter << std::endl;

  return 0;
}
