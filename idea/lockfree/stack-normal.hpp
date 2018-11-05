#include <mutex> 
class Stack {
 public:
  struct Node {
    Node* next;
    void* data;
  };

  void Push(Node* node) {
    std::lock_guard<std::mutex> lock(mutex_);
    node->next = head_;
    head_ = node;
  }

  Node* Pop() {
    std::lock_guard<std::mutex> lock(mutex_);
    Node* node = nullptr;
    if (head_) {
      node = head_;
      head_ = head_->next;
    }
    return node;
  }

 private:
  Node* head_ = nullptr;
  std::mutex mutex_;
};
