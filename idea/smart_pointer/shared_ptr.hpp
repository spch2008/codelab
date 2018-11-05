template<class T> 
class shared_ptr {
private:
  typedef shared_ptr<T> this_type;

public:
  typedef typename T element_type;

  shared_ptr() : px(0), pn() { }
  explicit shared_ptr(T* p): px(p), pn() {
    boost::detail::sp_pointer_construct(this, p, pn);
  }

  shared_ptr(shared_ptr const & r) : px(r.px), pn(r.pn) { }

  explicit shared_ptr(weak_ptr<T> const & r): pn(r.pn) {
    px = r.px;
  }

  shared_ptr(shared_ptr<T> const & r, element_type* p) : px(p), pn(r.pn) { }

  shared_ptr & operator=(shared_ptr const & r) {
    this_type(r).swap(*this);
    return *this;
  }

  shared_ptr(shared_ptr && r) : px(r.px), pn() {
    pn.swap(r.pn);
    r.px = 0;
  }

  shared_ptr& operator=(shared_ptr&& r) {
    this_type(static_cast<shared_ptr &&>(r)).swap(*this);
    return *this;
  }

  void reset(T * p) {
    this_type(p).swap(*this);
  }

  element_type& operator* () const {
    return *px;
  }
  
  element_type* operator-> () const {
    return px;
  }
  
  element_type * get() const {
    return px;
  }

  element_type * px;
  boost::detail::shared_count pn;

};

template <class T>
inline void sp_pointer_construct(
  boost::shared_ptr<T>* ppx, T* p, boost::detail::shared_count& pn) {
  boost::detail::shared_count(p).swap(pn);
  boost::detail::sp_enable_shared_from_this(ppx, p, p);
}

inline void sp_enable_shared_from_this(...) { }

template <class T>
inline void sp_enable_shared_from_this(
    boost::shared_ptr<T> const * ppx,
    T const * py,
    boost::enable_shared_from_this<T> const* pe) {
  if (pe != 0) {
    pe->_internal_accept_owner(ppx, const_cast< Y* >(py));
  }
}
