#include <mutex>
#include <atomic>
#include <vector>

template <typename T>
class DoublyBufferedData {
  class Wrapper;
public:
    class ScopedPtr {
     public:
      ScopedPtr() : data_(nullptr), w_(nullptr) {}
      ~ScopedPtr() {
        if (w_) { w_->EndRead(); }
      }

      const T* get() const { return data_; }
      const T& operator*() const { return *data_; }
      const T* operator->() const { return data_; }
        
    private:
      const T* data_;
      Wrapper* w_;
    };
    
    DoublyBufferedData();
    ~DoublyBufferedData();

    int Read(ScopedPtr* ptr);
    bool Modify(std::function<bool(T*)>&& fn);
    
private:
    const T* ReadImpl() const {
      return data_ + index_.load(std::memory_order_acquire);
    }

    Wrapper* AddWrapper();

    T data_[2];
    std::atomic<int> index_;

    pthread_key_t wrapper_key_;
    std::mutex wrapper_mutex_;
    std::vector<Wrapper*> wrappers_;
};

template <typename T>
class DoublyBufferedData<T>::Wrapper {
 public:
  inline void BeginRead() { mtx_.lock(); }
  inline void EndRead() { mtx_.unlock(); }
  inline void WaitReadDone() { std::unique_lock<std::mutex> lock(mtx_); }
    
 private:
  std::mutex mtx_;
};

template <typename T >
DoublyBufferedData<T>::DoublyBufferedData() : index_(0) {
  pthread_key_create(&wrapper_key_, nullptr);
}

template <typename T>
DoublyBufferedData<T>::~DoublyBufferedData() {
  pthread_key_delete(wrapper_key_);

  {
    std::unique_lock<std::mutex> lock(wrapper_mutex_);
    for (size_t i = 0; i < wrappers_.size(); ++i) {
      delete wrappers_[i];
    }
    wrappers_.clear();
  }
}

template <typename T>
int DoublyBufferedData<T>::Read(
    typename DoublyBufferedData<T>::ScopedPtr* ptr) {
  Wrapper* w = static_cast<Wrapper*>(pthread_getspecific(wrapper_key_));
  if (w != nullptr) {
    w->BeginRead();
    ptr->data_ = ReadImpl();
    ptr->w_ = w;
    return 0;
  }

  w = AddWrapper();
  if (w != nullptr && pthread_setspecific(wrapper_key_, w) == 0) {
    w->BeginRead();
    ptr->data_ = ReadImpl();
    ptr->w_ = w;
    return 0;
  }

  return -1;
}

template <typename T>
bool DoublyBufferedData<T>::Modify(std::function<bool(T*)>&& fn) {
  int bg_index = !index_.load(std::memory_order_relaxed);

  // 写后台数据
  const bool ret = fn(&data_[bg_index]);
  if (!ret) {
    return ret;
  }

  // 切换前后台
  index_.store(bg_index, std::memory_order_release);
  bg_index = !bg_index;
  
  // 确保旧数据都已失效
  {
    std::unique_lock<std::mutex> lock(wrapper_mutex_);
    for (size_t i = 0; i < wrappers_.size(); ++i) {
      wrappers_[i]->WaitReadDone();
    }
  }

  // 更新旧数据
  const bool ret2 = fn(&data_[bg_index]);
  return ret2;
}

template <typename T>
typename DoublyBufferedData<T>::Wrapper* DoublyBufferedData<T>::AddWrapper() {
  Wrapper* w = new Wrapper(this);
  if (nullptr == w) {
    return nullptr;
  }

  std::unique_lock<std::mutex> lock(wrapper_mutex_);
  wrappers_.push_back(w);
  return w;
}
