#include <atomic>
#include <new>
#include <glog/logging.h>

namespace ads_serving {
namespace dsp_util {

template <typename T>
class CircularQueue {
 public:
  CircularQueue() : head_(0), tail_(0) {}
  ~CircularQueue() { delete [] queue_; }

  bool Init(const int capacity = 1024) {
    if (capacity & (capacity - 1)) {
      LOG(ERROR) << "Invalid capacity=" << capacity
                 << " which must be power of 2";
      return false;
    }

    queue_ =  new(std::nothrow) T[capacity];
    if (queue_ == nullptr) {
      LOG(ERROR) << "Cann't malloc queue buffer.";
      return false;
    }

    capacity_ = capacity;
    capacity_mask_ = capacity - 1;
    return true;
  }

  // Push an item into the queue.
  // Never run in parallel with another push().
  bool Push(T item) {
    int curr_tail = tail_.load(std::memory_order_relaxed);
    int next_tail = capacity_mask_ & (curr_tail + 1);

    if(next_tail != head_.load(std::memory_order_acquire)) {
      queue_[curr_tail] = item;
      tail_.store(next_tail, std::memory_order_release);
      return true;
    }

    return false;
  }

  // Pop an item from the queue.
  // Never run in parallel with another pop().
  bool Pop(T* item) {
    int curr_head = head_.load(std::memory_order_relaxed);
    if(curr_head == tail_.load(std::memory_order_acquire)) {
      return false;
    }

    *item = queue_[curr_head];
    head_.store((curr_head + 1) & capacity_mask_, std::memory_order_release);
    return true;
  }

 private:
  T* queue_;
  int capacity_;
  int capacity_mask_;

  std::atomic<int> head_;
  std::atomic<int> tail_;
};
