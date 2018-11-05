#include <mutex>
#include <memory>
#include <unordered_map>

struct LRUHandle {
  LRUHandle* next;
  LRUHandle* prev;

  void* value;
  int key;

  bool in_cache_index;
  int refs;
};

class LRUCacheIndex {
 public:
  LRUHandle* Lookup(const int key);
  LRUHandle* Insert(const int key, LRUHandle* handle);
  LRUHandle* Erase(const int key);

 private:
  std::unordered_map<int, LRUHandle*> index_;
};

class LRUCache {
 public:
  explicit LRUCache(const size_t capacity);
  ~LRUCache();

  bool Insert(const int key, void* value);
  LRUHandle* Lookup(const int key);

  void Erase(const int key);
  void Release(LRUHandle* handle);

 private:
  void LRU_Remove(LRUHandle* e);
  void LRU_Append(LRUHandle*list, LRUHandle* e);
  void Ref(LRUHandle* e);
  void Unref(LRUHandle* e);
  bool RemoveFromLRU(LRUHandle* e);

  std::mutex mutex_;
  size_t usage_;
  size_t capacity_;

  LRUHandle lru_;
  LRUHandle in_use_;
  LRUCacheIndex cache_index_;
};

LRUCache::LRUCache(const size_t capacity)
    : usage_(0), capacity_(capacity) {
  lru_.next = &lru_;
  lru_.prev = &lru_;
  in_use_.next = &in_use_;
  in_use_.prev = &in_use_;
}

LRUCache::~LRUCache() {
  //assert(in_use_.next == &in_use_); // 有未释放的。
  for (LRUHandle* e = lru_.next; e != &lru_; ) {
    LRUHandle* next = e->next;
    e->in_cache_index = false;
    Unref(e);
    e = next;
  }
}

bool LRUCache::Insert(const int key, void* value) {
  std::unique_ptr<LRUHandle> e(new LRUHandle);
  e->value = value;
  e->key = key;

  std::unique_lock<std::mutex> l(mutex_);
  while (usage_ >= capacity_ && lru_.next != &lru_) {
    LRUHandle* old = lru_.next;
    RemoveFromLRU(cache_index_.Erase(old->key));
  }

  if (usage_ < capacity_) {
    usage_ += 1;
    LRU_Append(&lru_, e.get());

    LRUHandle* old_same_key_handle = cache_index_.Insert(key, e.release());
    e->in_cache_index = true;
    e->refs = 1; // due to exist in cache_index.

    RemoveFromLRU(old_same_key_handle);
    return true;
  }

  return false;
}

void LRUCache::Ref(LRUHandle* e) {
  // First, on lru_ list, move to in_use_ list.
  if (e->refs == 1 && e->in_cache_index) {
    LRU_Remove(e);
    LRU_Append(&in_use_, e);
  }
  e->refs++;
}

void LRUCache::Unref(LRUHandle* e) {
  e->refs--;
  if (e->refs == 0) {
    delete e;
  } else if (e->in_cache_index && e->refs == 1) {
    // Move from ins_use_ list to lru_ list.
    LRU_Remove(e);
    LRU_Append(&lru_, e);
  }
}

LRUHandle* LRUCache::Lookup(const int key) {
  std::unique_lock<std::mutex> l(mutex_);
  LRUHandle* e = cache_index_.Lookup(key);
  if (e != NULL) {
    Ref(e);
  }

  return e; 
}

void LRUCache::Release(LRUHandle* handle) {
  std::unique_lock<std::mutex> l(mutex_);
  Unref(handle);
}

void LRUCache::Erase(const int key) {
  std::unique_lock<std::mutex> l(mutex_);
  // 从索引中删除就找不到了，等全部释放后即删除。
  RemoveFromLRU(cache_index_.Erase(key));
}

void LRUCache::LRU_Remove(LRUHandle* e) {
  e->next->prev = e->prev;
  e->prev->next = e->next;
}

void LRUCache::LRU_Append(LRUHandle* list, LRUHandle* e) {
  e->next = list;
  e->prev = list->prev;
  e->prev->next = e;
  e->next->prev = e;
}

bool LRUCache::RemoveFromLRU(LRUHandle* e) {
  if (e != NULL) {
    LRU_Remove(e);
    e->in_cache_index = false;
    Unref(e);
  }
  return e != NULL;
}

LRUHandle* LRUCacheIndex::Lookup(const int key) {
  const auto iter = index_.find(key);
  if (iter != index_.end()) {
    return iter->second;
  }
  return nullptr;
}

LRUHandle* LRUCacheIndex::Insert(const int key, LRUHandle* handle) {
  LRUHandle* old_handle = Lookup(key);
  if (old_handle) {
    index_.erase(key);
  }

  index_[key] = handle;
  return old_handle;
}

LRUHandle* LRUCacheIndex::Erase(const int key) {
  LRUHandle* old_handle = Lookup(key);
  index_.erase(key);
  return old_handle;
}
