#include <atomic>
using namespace std;
#include "tag_ptr.h"

template <typename T>
class queue {
 public:
  struct node {
    atomic<TagPtr<node>> next;
    T* data;
  };
  
  queue() {
    TagPtr<node> dummy_node(new node, 0);
    head_.store(dummy_node, memory_order_relaxed);
    tail_.store(dummy_node, memory_order_release);
  }
  
  ~queue() {
    while (pop()) {}
    delete head_.load(memory_order_relaxed).GetPtr();
  }
  
  bool push(const T& data) {
    node* new_node = GetNode(data); // next remain tag, but ptr set null.
    for (;;) {
      TagPtr<node> old_tail = tail_.load(memory_order_acquire);
      TagPtr<node> old_tail_next = old_tail->next.load(memory_order_acquire);
  
      if (old_tail_next.GetPtr() == nullptr) {
        TagPtr<node> new_tail_next(new_node, old_tail_next.GetNextTag());
        if (old_tail->next.compare_exchange_weak(old_tail_next, new_tail_next)) {
          TagPtr<node> new_tail(new_node, old_tail.GetNextTag());
          tail_.compare_exchange_strong(old_tail, new_tail);
          return true;
        }
      } else {
        TagPtr<node> new_tail(old_tail_next.GetPtr(), old_tail.GetNextTag());
        tail_.compare_exchange_strong(old_tail, new_tail);
      }
    }
  }
  
  T* pop () {
    T* ret = nullptr;
    for (;;) {
      TagPtr<node> old_head = head_.load(memory_order_acquire);
      TagPtr<node> old_head_next = old_head->next.load(memory_order_acquire);
  
      TagPtr<node> old_tail = tail_.load(memory_order_acquire);
      if (old_tail.GetPtr() == old_head.GetPtr()) {
        if (old_head_next.GetPtr() == nullptr) {
          break;
        }

        TagPtr<node> new_tail(old_head_next.GetPtr(), old_tail.GetNextTag());
        tail_.compare_exchange_string(old_tail, new_tail);
      } else {
        TagPtr<node> new_head(old_head_next.GetPtr(), old_head.GetNextTag());
        if (head_.compare_exchange_weak(old_head, new_head)) {
          ret = old_head->value;
          Destruct(old_head);
          break;
        }
      }
    }
    return ret;
  }

 private:
  atomic<TagPtr<node>> head_;
  atomic<TagPtr<node>> tail_;
};
