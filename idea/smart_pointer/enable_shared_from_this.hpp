template<class T>
class enable_shared_from_this {
 protected:
  enable_shared_from_this() { }
  ~enable_shared_from_this() { }
  enable_shared_from_this(enable_shared_from_this const &) { }

  enable_shared_from_this& operator=(enable_shared_from_this const &) {
    return *this;
  }

 public:
  shared_ptr<T> shared_from_this() {
    shared_ptr<T> p( weak_this_ );
    return p;
  }

 public: 
  template<class X, class Y>
  void _internal_accept_owner( shared_ptr<X> const * ppx, Y * py ) const {
    if( weak_this_.expired() ) {
      weak_this_ = shared_ptr<T>( *ppx, py );
    }
  }

private:
  mutable weak_ptr<T> weak_this_;
};
