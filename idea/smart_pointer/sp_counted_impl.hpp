
void atomic_increment(atomic<int>* pw) {
  pw->fetch_add(1, memory_order_relaxed);
}

int atomic_decrement(atomic<int> * pw) {
  return pw->fetch_sub(1, memory_order_acq_rel);
}

int atomic_conditional_increment(atomic<int> * pw) {
  // long r = *pw;
  // if( r != 0 ) ++*pw;
  // return r;

  int r = pw->load(memory_order_relaxed);

  for( ;; ) {
    if( r == 0 ) {
      return r;
    }

    if(pw->compare_exchange_weak(r, r + 1, memory_order_relaxed, memory_order_relaxed)) {
      return r;
    }
  }    
}

class sp_counted_base {
 private:
    sp_counted_base(sp_counted_base const &);
    sp_counted_base & operator=(sp_counted_base const &);

    std::atomic<int> use_count_;
    std::atomic<int> weak_count_;

 public:
  sp_counted_base(): use_count_(1), weak_count_(1) { }
  virtual ~sp_counted_base() { }

  // dispose() is called when use_count_ drops to zero, to release
  // the resources managed by *this.
  virtual void dispose() = 0;

  // destroy() is called when weak_count_ drops to zero.
  virtual void destroy() {
      delete this;
  }

  void add_ref_copy() {
    atomic_increment(&use_count_);
  }

  bool add_ref_lock() {
    return atomic_conditional_increment(&use_count_) != 0;
  }

  void release() {
    if(atomic_decrement(&use_count_) == 1) {
      dispose();
      weak_release(); // 把初始创建的weak_count 清0
    }
  }

  void weak_add_ref() {
    atomic_increment(&weak_count_);
  }

  void weak_release() {
    if(atomic_decrement( &weak_count_ ) == 1) {
      destroy();
    }
  }

  long use_count() const {
    return use_count_.load( std::memory_order_acquire );
  }
};

///////////////////////////////////////////////////////////////////

template<class X>
class sp_counted_impl_p: public sp_counted_base {
private:

    X * px_;

    sp_counted_impl_p( sp_counted_impl_p const & );
    sp_counted_impl_p & operator= ( sp_counted_impl_p const & );

    typedef sp_counted_impl_p<X> this_type;

public:

    explicit sp_counted_impl_p( X * px ): px_( px )
    {
    }

    virtual void dispose() // nothrow
    {
        boost::checked_delete( px_ );
    }
};
