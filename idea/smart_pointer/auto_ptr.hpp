#ifndef AUTO_PTR_H
#define AUTO_PTR_H

template<typename T>
class auto_ptr {
 private:
  T* ptr_;

 public:
  typedef T element_type;

  explicit auto_ptr(element_type* p = 0) throw() : ptr_(p) { }
  auto_ptr(auto_ptr& a) throw() : ptr_(a.release()) { }

  auto_ptr& operator=(auto_ptr& a) throw() {
    reset(a.release());
    return *this;
  }

  ~auto_ptr() { delete ptr_; }

  element_type& operator*() const throw() {
    return *ptr_; 
  }

  element_type* operator->() const throw() {
    return ptr_; 
  }

  element_type* get() const throw() { return ptr_; }


  element_type* release() throw() {
    element_type* tmp = ptr_;
    ptr_ = 0;
    return tmp;
  }

  void reset(element_type* p = 0) throw() {
    if (p != ptr_) {
      delete ptr_;
      ptr_ = p;
    }
  }
};

#endif
