#include <atomic>
#include <thread>
#include <iostream>
#include <sstream>

struct lock_t {
  static const size_t max_locks = 5;
  std::atomic<int> locks[max_locks];

  bool lock(size_t const thread_id) {

    locks[thread_id].store(1, std::memory_order_release);    // Store
    //locks[thread_id].store(1, std::memory_order_seq_cst);    // Store

    for (size_t i = 0; i < max_locks; ++i)
      if (locks[i].load(std::memory_order_acquire) > 0 && i != thread_id) { // Load
          locks[thread_id].store(0, std::memory_order_release);   // undo lock
        return false;
      }
    return true;
  }

  void unlock(size_t const thread_id) {
    locks[thread_id].store(0, std::memory_order_release);
  }
};

lock_t custom_lock;
int counter = 0;

void test(size_t const thread_id)  {
  for (size_t i = 0; i < 100000; ++i) {
    while (!custom_lock.lock(thread_id));
    ++counter;
    custom_lock.unlock(thread_id);
  }
}

int main() {
  counter = 0;
  std::thread t0([&]() { test(0); });
  std::thread t1([&]() { test(1); });
  t0.join(); t1.join();

  std::cout << counter << std::endl;

  return 0;
}
