/**
* @file   thread_pool.h
* @author sunpengcheng(spch2008n@foxmail.com)
* @date   2018-10-30 13:32:27
* @brief
**/

#include <chrono>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>

class ThreadPool {
 public:
  ThreadPool(int pool_size) : stop_(false) {
    for (int i = 0; i < pool_size; ++i) {
      threads_.emplace_back(std::thread(std::bind(&ThreadPool::WorkerHandler, this)));
    }
  }

  ~ThreadPool() {
    {
      std::lock_guard<std::mutex> lock(mtx_);
      stop_ = true;
    }

    cv_.notify_all();

    for (size_t i = 0; i < threads_.size(); ++i) {
      threads_[i].join();
    }
  }

  void AddTask(std::function<void()>&& task) {
    {
      std::lock_guard<std::mutex> lock(mtx_);
      worker_queue_.push(std::move(task));
    }

    cv_.notify_one();
  }

 private:
  void WorkerHandler() {
    while (true) {
      std::unique_lock<std::mutex> lock(mtx_);
      cv_.wait(lock, [this]() { return this->stop_ || !this->worker_queue_.empty(); });

      if (stop_) {
        break;
      }

      std::function<void()> task = std::move(worker_queue_.front());
      worker_queue_.pop();

      lock.unlock();
      task();
    }
  }

  bool stop_;
  std::mutex mtx_;
  std::condition_variable cv_;

  std::vector<std::thread> threads_;
  std::queue<std::function<void()>> worker_queue_;
};


void func(int n) {
  std::this_thread::sleep_for(std::chrono::seconds(n));
  std::cout << "wake for: " << n << " thread_id: " << std::this_thread::get_id() << std::endl;
}

int main() {
  ThreadPool tl(10);

  for (int i = 1; i <= 20; ++i) {
    tl.AddTask(std::bind(func, 2));
  }

  std::this_thread::sleep_for(std::chrono::seconds(50));
}
