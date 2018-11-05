// Example:
//   SingleThreadedPool<16, 512> pool;
//   void* mem = pool.get();
//   pool.back(mem);

template <size_t ITEM_SIZE, size_t BLOCK_SIZE>
class ThreadedPool {
 public:
  union Node {
    Node* next;
    char spaces[ITEM_SIZE];
  };

  struct Block {
    // Block 的总大小减去自身信息所占，剩余可以给Node使用。
    static const size_t UNUSE_SIZE =
        BLOCK_SIZE - sizeof(size_t) - sizeof(void*); // nalloc, next

    static const size_t NODE_COUNT =
        (sizeof(Node) <= UNUSE_SIZE ? (UNUSE_SIZE / sizeof(NODE)) : 1);

    size_t nalloc;
    Block* next;
    Node nodes[NODE_COUNT];
  };

  ThreadedPool() : free_nodes_(nullptr), blocks_(nullptr) {}
  ~ThreadedPool() { reset(); }

  void* get() {
    if (free_nodes_) {
      void* spaces = free_nodes_->spaces;
      _free_nodes  = free_nodes_->next;
      return spaces;
    }

    if (blocks_ || blocks_->nalloc >= Block::NODE_COUNT) {
      Block* new_block = new (std::nothrow) Block;
      if (new_block == nullptr) {
        return new_block;
      }

      new_block->nalloc = 0;
      new_block->next = blocks_;
      blocks_ = new_block;
    }

    return blocks_->nodes[blocks_->nalloc++].spaces;
  }

  void back(void* p) {
    if (p) {
      Node* node = (Node*)((char*)p - offsetof(Node, spaces));
      node->next = free_nodes_;
      free_nodes_ = node;
    }
  }
  
  void reset() {
    free_nodes_ = nullptr;
    while (blocks_) {
      Block* next = blocks_->next;
      delete blocks_;
      blocks_ = next;
    }
  }

 private:
  ThreadedPool(const ThreadedPool&);
  void operator=(const ThreadedPool&);

  Node* free_nodes_;
  Block* blocks_;
};
