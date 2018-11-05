/**
* @file   socket_pool.cpp
* @author sunpengcheng(spch2008n@foxmail.com)
* @date   2018-10-31 13:00:15
* @brief
**/
#include <unistd.h>
#include <atomic>
#include <mutex>
#include <vector>

class Socket {
 public:
  Socket(const char* ip, const int port)
        : socket_fd_(-1)
        , error_code_(0) {
    // create socket
  }

  ~Socket() {
    if (socket_fd_ >= 0) { close(socket_fd_); }
  }

  void SetFailed(const int error_code) { error_code_ = error_code; }
  bool IsValid() { return socket_fd_ >= 0 && error_code_ == 0; }

 private:
  int socket_fd_;
  int error_code_;
};

class SocketPool {
 public:
  struct EndPoint {
    const char* ip;
    int port;
  };

  SocketPool(const EndPoint& ep, const int pool_size)
    : remote_side_(ep)
    , pool_size_(pool_size)
    , pool_count_(0) {}
  ~SocketPool();

  Socket* GetSocket();
  void ReturnSocket(Socket* sock);

 private:
  EndPoint remote_side_;
  const int pool_size_;

  std::mutex mtx_;
  std::atomic<int> pool_count_;
  std::vector<Socket*> pool_;
};

SocketPool::~SocketPool() {
  std::unique_lock<std::mutex> lock(mtx_);
  for (size_t i = 0; i < pool_.size(); ++i) {
    delete pool_[i];
  }

  pool_.clear();
}

Socket* SocketPool::GetSocket() {
  for ( ; ; ) {
    Socket* sock = nullptr;
    {
      std::unique_lock<std::mutex> lock(mtx_);
      if (pool_.empty()) {
        break;
      }

      sock = pool_.back();
      pool_.pop_back();
    }

    pool_count_.fetch_sub(1, std::memory_order_relaxed);

    if (sock && sock->IsValid()) {
      return sock;
    }

    delete sock;
  }

  if (pool_count_.load(std::memory_order_relaxed) < pool_size_) {
    pool_count_.fetch_add(1, std::memory_order_relaxed);
    return new Socket(remote_side_.ip, remote_side_.port);
  }

  return nullptr;
}

void SocketPool::ReturnSocket(Socket* sock) {
  if (sock == nullptr) {
    return;
  }

  {
    std::unique_lock<std::mutex> lock(mtx_);
    pool_.push_back(sock);
  }

  pool_count_.fetch_add(1, std::memory_order_relaxed);
}
