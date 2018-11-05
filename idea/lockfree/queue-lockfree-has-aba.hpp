#include <atomic>
using namespace std;

template <typename T>
class queue {
 public:
  struct node {
    atomic<node*> next;
    T* data;
  };
  
  queue() {
    node* dummy_node = new node;
    head_.store(dummy_node, memory_order_relaxed);
    tail_.store(dummy_node, memory_order_release);
  }
  
  ~queue() {
    while (pop()) {}
    delete head_.load(memory_order_relaxed);
  }
  
  bool push(node* new_node) {
    for (;;) {
      node* old_tail = tail_.load(memory_order_acquire);
      node* old_tail_next = old_tail->next.load(memory_order_acquire);
  
      if (old_tail_next == nullptr) {
        if (old_tail->next.compare_exchange_weak(old_tail_next, new_node)) {
          tail_.compare_exchange_strong(old_tail, new_node);
          return true;
        }
      } else {
        tail_.compare_exchange_strong(old_tail, old_tail_next);
      }
    }
  }
  
  T* pop () {
    T* ret = nullptr;
    for (;;) {
      node* old_head = head_.load(memory_order_acquire);
      node* old_head_next = old_head->next.load(memory_order_acquire);
  
      node* old_tail = tail_.load(memory_order_acquire);
      if (old_tail == old_head) {
        if (old_head_next == nullptr) {
          break;
        }

        tail_.compare_exchange_strong(old_tail, old_head_next);
      } else {
        node* new_head = old_head_next;
        if (head_.compare_exchange_weak(old_head, new_head)) {
          ret = old_head->value;
          delete old_head;
          break;
        }
      }
    }
    return ret;
  }

 private:
  atomic<node*> head_;
  atomic<node*> tail_;
};
