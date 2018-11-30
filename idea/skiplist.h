
class SkipList {
 public:
  SkipList();

  void Insert(const int key);
  bool Contains(const Key& key) const;

 private:
  enum { kMaxHeight = 12 };

  Node* const head_;

  int max_height_;

  inline int GetMaxHeight() const {
    return max_height_;
  }

  Node* NewNode(const int key, int height);
  int RandomHeight();

  // Return the earliest node that comes at or after key.
  // Return NULL if there is no such node.
  //
  // If prev is non-NULL, fills prev[level] with pointer to previous
  // node at "level" for every level in [0..max_height_-1].
  Node* FindGreaterOrEqual(const Key& key, Node** prev) const;

  // No copying allowed
  SkipList(const SkipList&);
  void operator=(const SkipList&);
};

struct SkipList::Node {
  explicit Node(const int k) : key(k) { }

  int const key;

  Node* Next(int n) {
    return next_[n];
  }

  void SetNext(int n, Node* x) {
    next_[n] = x;
  }

 private:
  Node* next_[1];
};


int SkipList::RandomHeight() {
  return [1, kMaxHeight];
}

SkipList::Node* SkipList::FindGreaterOrEqual(const int key, Node** prev) const {
  Node* x = head_;
  int level = GetMaxHeight() - 1;

  while (true) {
    Node* next = x->Next(level);
    if (next && key >= next->key) {
      // Keep searching in this list
      x = next;
    } else {
      if (prev != NULL) prev[level] = x;
      if (level == 0) {
        return next;
      } else {
        // Switch to next list
        level--;
      }
    }
  }
}

SkipList::Node* SkipList::FindLessThan(const int key, Node** prev) const {
  Node* x = head_;
  int level = GetMaxHeight() - 1;
  while (true) {
    Node* next = x->Next(level);
    if (next && key < next->key) {
      // Keep searching in this list
      x = next;
    } else {
      if (prev != NULL) prev[level] = x;
      if (level == 0) {
        return next;
      } else {
        level--;
      }
    }
  }
}

SkipList::SkipList() {
      : head_(NewNode(0, kMaxHeight)),
      , max_height_(1) {
  for (int i = 0; i < kMaxHeight; i++) {
    head_->SetNext(i, NULL);
  }
}

void SkipList::Insert(const int key) {
  Node* prev[kMaxHeight];
  Node* x = FindGreaterOrEqual(key, prev);

  int height = RandomHeight();
  if (height > GetMaxHeight()) { // 大于当前的高度
    for (int i = GetMaxHeight(); i < height; i++) {
      prev[i] = head_;
    }

    max_height_ = height;
  }

  x = NewNode(key, height);
  for (int i = 0; i < height; i++) {
    x->SetNext(i, prev[i]->Next(i));
    prev[i]->SetNext(i, x);
  }
}

bool SkipList::Contains(const int key) const {
  Node* x = FindGreaterOrEqual(key, NULL);
  if (x != NULL && key == x->key) {
    return true;
  } else {
    return false;
  }
}

SkipList::Node* SkipList::NewNode(const int key, int height) {
  char* mem = malloc(
      sizeof(Node) + sizeof(int) * (height - 1));
  return new (mem) Node(key);
}
