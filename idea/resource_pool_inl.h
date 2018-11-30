static const size_t RP_MAX_BLOCK_NGROUP = 65536; // MAX SIZE FOR BLOCK GROUP

static const size_t RP_GROUP_NBLOCK_NBIT = 16;
static const size_t RP_GROUP_NBLOCK = (1UL << RP_GROUP_NBLOCK_NBIT);
static const size_t RP_INITIAL_FREE_LIST_SIZE = 1024;

struct ResourceId {
    uint64_t value;

  operator uint64_t() const {
    return value;
  }
};

template <typename T, size_t NITEM> 
struct ResourcePoolFreeChunk {
    size_t nfree;
    ResourceId<T> ids[NITEM];
};

template <typename T>
class ResourcePool {
public:
    static const size_t BLOCK_NITEM = 128;
    static const size_t FREE_CHUNK_NITEM = BLOCK_NITEM;

    typedef ResourcePoolFreeChunk<T, FREE_CHUNK_NITEM>     FreeChunk;
    typedef ResourcePoolFreeChunk<T, 0> DynamicFreeChunk;

    struct Block {
      //T items[N]; // 这样相当于已经创建了
      char items[sizeof(T) * BLOCK_NITEM];
      size_t nitem;
      Block() : nitem(0) {}
    };

    struct BlockGroup {
      std::atomic<size_t> nblock;
      std::atomic<Block*> blocks[RP_GROUP_NBLOCK];

      BlockGroup() : nblock(0) {
        memset(blocks, 0, sizeof(butil::atomic<Block*>) * RP_GROUP_NBLOCK);
      }
    };

    inline T* get_resource(ResourceId<T>* id) {
      LocalPool* lp = get_or_new_local_pool();
      if (lp != NULL) {
        return lp->get(id);
      }
      return NULL;
    }

    inline int return_resource(ResourceId<T> id) {
      LocalPool* lp = get_or_new_local_pool();
      if (lp != NULL) {
        return lp->return_resource(id);
      }
      return -1;
    }

    static inline ResourcePool* singleton() {
      ResourcePool* p = _singleton.load(butil::memory_order_consume);
      if (p) {
        return p;
      }

      std::lock_guard<std::mutex> lock(_singleton_mutex);
      p = _singleton.load(butil::memory_order_consume);
      if (!p) {
        p = new ResourcePool();
        _singleton.store(p, butil::memory_order_release);
      } 

      return p;
    }

private:
    ResourcePool() {
      _free_chunks.reserve(RP_INITIAL_FREE_LIST_SIZE);
    }

    ~ResourcePool() { }

    inline LocalPool* get_or_new_local_pool() {
      LocalPool* lp = _local_pool;
      if (lp != NULL) {
          return lp;
      }

      lp = new (std::nothrow) LocalPool(this);
      if (NULL == lp) {
          return NULL;
      }

      return lp;
    }

    bool pop_free_chunk(FreeChunk& c) {
      if (_free_chunks.empty()) {
        return false;
      }

      _free_chunks_mutex.lock();
      if (_free_chunks.empty()) {
        _free_chunks_mutex.unlock();
        return false;
      }

      DynamicFreeChunk* p = _free_chunks.back();
      _free_chunks.pop_back();
      _free_chunks_mutex.unlock();

      c.nfree = p->nfree;
      memcpy(c.ids, p->ids, sizeof(*p->ids) * p->nfree);
      free(p);
      return true;
    }

    bool push_free_chunk(const FreeChunk& c) {
      DynamicFreeChunk* p = (DynamicFreeChunk*)malloc(
          offsetof(DynamicFreeChunk, ids) + sizeof(*c.ids) * c.nfree);
      if (!p) {
        return false;
      }

      p->nfree = c.nfree;
      memcpy(p->ids, c.ids, sizeof(*c.ids) * c.nfree);

      _free_chunks_mutex.lock();
      _free_chunks.push_back(p);
      _free_chunks_mutex.unlock();
      return true;
    }

    Block* fetch_block(size_t* index) {
      Block* const new_block = new (std::nothrow) Block;
      if (new_block == NULL) {
        return NULL;
      }

      while (true) {
        size_t ngroup = _ngroup.load(std::memory_order_acquire);
        if (ngroup >= 1) {
          BlockGroup* const g =
              _block_groups[ngroup - 1].load(std::memory_order_acquire);

          const size_t block_index = g->nblock.fetch_add(1, std::memory_order_relaxed);
          if (block_index < RP_GROUP_NBLOCK) {
            g->blocks[block_index].store(
                new_block, std::memory_order_release);
            *index = (ngroup - 1) * RP_GROUP_NBLOCK + block_index;
            return new_block;
          }

          g->nblock.fetch_sub(1, std::memory_order_relaxed);
        }

        if (!create_block_group(ngroup)) {
          break;
        }
      }

      delete new_block;
      return NULL;
    }

    bool create_block_group(size_t old_ngroup) {
      BlockGroup* bg = NULL;
      std::lock_guard<std::mutex> lock(_blogk_group_mutex);

      const size_t ngroup = _ngroup.load(butil::memory_order_acquire);
      if (ngroup != old_ngroup) {
        // Other thread got lock and added group before this thread.
        return true;
      }

      if (ngroup < RP_MAX_BLOCK_NGROUP) {
        bg = new(std::nothrow) BlockGroup;
        if (NULL != bg) {
          _block_groups[ngroup].store(bg, butil::memory_order_release);
          _ngroup.store(ngroup + 1, butil::memory_order_release);
        }
      }
      return bg != NULL;
    }

 private:
  static std::mutex _singleton_mutex;
  static std::atomic<ResourcePool*> _singleton;

  std::mutex _block_groups_mutex;
  std::atomic<size_t> _block_groups_size;
  std::vector<BlockGroup*> _block_groups;

  std::mutex _free_chunks_mutex;
  std::vector<DynamicFreeChunk*> _free_chunks;

  static __thread LocalPool* _local_pool;
};

class LocalPool {
 public:
  explicit LocalPool(ResourcePool* pool)
      : _pool(pool)
      , _cur_block(NULL)
      , _cur_block_index(0) {
    _cur_free.nfree = 0;
  }

  ~LocalPool() {
    if (_cur_free.nfree) {
      _pool->push_free_chunk(_cur_free);
    }

    // 归还block
  }

  inline T* get(ResourceId<T>* id) {
    /* Fetch local free id */                                       
    if (_cur_free.nfree) {                                          
        const ResourceId<T> free_id = _cur_free.ids[--_cur_free.nfree]; 
        *id = free_id;                                              
        return unsafe_address_resource(free_id);                    
    }                                                               

    /* Fetch a FreeChunk from global.*/
    if (_pool->pop_free_chunk(_cur_free)) {
        --_cur_free.nfree;
        const ResourceId<T> free_id = _cur_free.ids[_cur_free.nfree];
        *id = free_id;                                              
        return unsafe_address_resource(free_id);
    }

    /* Fetch memory from local block */                             
    if (_cur_block && _cur_block->nitem < BLOCK_NITEM) {            
        id->value = _cur_block_index * BLOCK_NITEM + _cur_block->nitem; 
        T* p = new ((T*)_cur_block->items + _cur_block->nitem) T; 
        ++_cur_block->nitem;                                        
        return p;                                                   
    }

    /* Fetch a Block from global */                                 
    _cur_block = add_block(&_cur_block_index);                      
    if (_cur_block != NULL) {                                       
        id->value = _cur_block_index * BLOCK_NITEM + _cur_block->nitem; 
        T* p = new ((T*)_cur_block->items + _cur_block->nitem) T; 
        ++_cur_block->nitem;                                        
        return p;                                                   
    }                                                               
    return NULL;                                                    
  }

  inline int return_resource(ResourceId<T> id) {
    // Return to local free list
    if (_cur_free.nfree < ResourcePool::free_chunk_nitem()) {
        _cur_free.ids[_cur_free.nfree++] = id;
        return 0;
    }
    // Local free list is full, return it to global.
    // For copying issue, check comment in upper get()
    if (_pool->push_free_chunk(_cur_free)) {
        _cur_free.nfree = 1;
        _cur_free.ids[0] = id;
        return 0;
    }
    return -1;
  }

private:
  ResourcePool* _pool;
  FreeChunk _cur_free;

  Block* _cur_block;
  size_t _cur_block_index;
};

/*
static inline T* unsafe_address_resource(ResourceId<T> id) {
    const size_t block_index = id.value / BLOCK_NITEM;
    return (T*)(_block_groups[(block_index >> RP_GROUP_NBLOCK_NBIT)]
                .load(butil::memory_order_consume)
                ->blocks[(block_index & (RP_GROUP_NBLOCK - 1))]
                .load(butil::memory_order_consume)->items) +
           id.value - block_index * BLOCK_NITEM;
}

static inline T* address_resource(ResourceId<T> id) {
    const size_t block_index = id.value / BLOCK_NITEM;
    const size_t group_index = (block_index >> RP_GROUP_NBLOCK_NBIT);
    if (__builtin_expect(group_index < RP_MAX_BLOCK_NGROUP, 1)) {
        BlockGroup* bg =
            _block_groups[group_index].load(butil::memory_order_consume);
        if (__builtin_expect(bg != NULL, 1)) {
            Block* b = bg->blocks[block_index & (RP_GROUP_NBLOCK - 1)]
                       .load(butil::memory_order_consume);
            if (__builtin_expect(b != NULL, 1)) {
                const size_t offset = id.value - block_index * BLOCK_NITEM;
                if (__builtin_expect(offset < b->nitem, 1)) {
                    return (T*)b->items + offset;
                }
            }
        }
    }

    return NULL;
}
*/

