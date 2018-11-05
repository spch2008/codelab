template<class T>
class weak_ptr {
 private:
    typedef weak_ptr<T> this_type;

 public:
    typedef T element_type;

    weak_ptr() : px(0), pn() { }

    weak_ptr(weak_ptr const & r) : px(r.px), pn(r.pn) { }
    weak_ptr& operator=(weak_ptr const & r) {
      px = r.px;
      pn = r.pn;
      return *this;
    }

    template<class Y>
    weak_ptr(shared_ptr<Y> const& r) : px( r.px ), pn( r.pn ) { }

    template<class Y>
    weak_ptr & operator=(shared_ptr<Y> const & r ) {
      px = r.px;
      pn = r.pn;

      return *this;
    }

    shared_ptr<T> lock() const {
      return shared_ptr<T>(*this, boost::detail::sp_nothrow_tag());
    }

private:
  element_type* px;
  boost::detail::weak_count pn;
};
