#include <mutex> 
#include <atomic>
using namespace std;

class Stack {
 public:
  struct Node {
    Node* next;
    void* data;
  };

  void Push(Node* node) {
    Node* old_head = head_.load(memory_order_acquire);
    for (;;) {
      node->next = old_head;
      if (head_.compare_exchange_weak(old_head, node)) {
        break;
      }
    }
  }

  Node* Pop() {
    Node* old_head = head_.load(memory_order_acquire);
    for (;;) {
      if (old_head == nullptr) {
        break;
      }

      Node* new_head = old_head->next;
      if (head_.compare_exchange_weak(old_head, new_head)) {
        break;
      }
    }

    return old_head;
  }

 private:
  atomic<Node*> head_;
};
