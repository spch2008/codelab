/**
* @file   tag_ptr.h
* @author sunpengcheng(spch2008n@foxmail.com)
* @date   2018-09-27 09:25:20
* @brief
**/

#include <mutex> 
#include <atomic>
using namespace std;

template <typename T>
class TagPtr {
 public:
  typedef unsigned long long int uint64;
  typedef unsigned short uint16;
  typedef int int32;

  TagPtr() : compress_ptr_(PackPtr(nullptr, 0)) {}
  TagPtr(T* ptr, int version = 0) : compress_ptr_(PackPtr(ptr, version)) {}

  T* GetPtr() const { return reinterpret_cast<T*>(compress_ptr_ & ptr_mask); }
  int32 GetTag() const { return ExtractTag(compress_ptr_); }
  int32 GetNextTag() const {
      return (GetTag() + 1) & std::numeric_limits<uint16>::max();
  }

  void SetTag(int32 version) {
    CastUnit cu;
    cu.value = compress_ptr_;
    cu.tag[tag_index] = version;
    compress_ptr_ = cu.value;
  }

  T& operator*() const { return *GetPtr(); }   
  T* operator->() const { return GetPtr(); }   
  operator bool(void) const { return GetPtr() != nullptr; }

 private:
  union CastUnit {
    uint64 value;
    uint16 tag[4];
  };

  static const int tag_index = 3;
  static const uint64 ptr_mask = 0xffffffffffffUL;

  uint64 PackPtr(T* ptr, int32 version) {
    CastUnit cu;
    cu.value = reinterpret_cast<uint64>(ptr);
    cu.tag[tag_index] = version;
    return cu.value;
  }

  int32 ExtractTag(const uint64 compress_ptr) const {
    CastUnit cu;
    cu.value = compress_ptr;
    return cu.tag[tag_index];
  }

  uint64 compress_ptr_;
};
