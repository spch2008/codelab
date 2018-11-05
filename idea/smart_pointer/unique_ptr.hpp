template<typename T>
class unique_ptr {
 private:
  T* ptr_;

  unique_ptr(unique_ptr& a);
  unique_ptr& operator=(unique_ptr& a);

 public:
  typedef T element_type;

  explicit unique_ptr(element_type* p = 0) : ptr_(p) { }
  unique_ptr(unique_ptr&& u) : ptr_(u.release()) {}

  unique_ptr& operator=(unique_ptr&& u) {
    reset(u.release());
    return *this;
  }

  ~unique_ptr() { delete ptr_; }

  element_type& operator*() const {
    return *ptr_; 
  }

  element_type* operator->() const {
    return ptr_; 
  }

  element_type* get() const { return ptr_; }

  element_type* release() {
    element_type* tmp = ptr_;
    ptr_ = 0;
    return tmp;
  }

  void reset(element_type* p = 0) {
    if (p != ptr_) {
      delete ptr_;
      ptr_ = p;
    }
  }
};
