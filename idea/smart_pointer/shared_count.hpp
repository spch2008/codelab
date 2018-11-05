class shared_count {
 private:
  sp_counted_base * pi_;

 public:
  shared_count(): pi_(0) { }

  template<class Y> 
  explicit shared_count(Y * p): pi_( 0 ) {
    try {
      pi_ = new sp_counted_impl_p<Y>(p);
    } catch(...) {
      boost::checked_delete( p );
      throw;
    }
  }

  ~shared_count() {
    if( pi_ != 0 ) pi_->release();
  }

  shared_count(shared_count const & r): pi_(r.pi_) {
    if( pi_ != 0 ) pi_->add_ref_copy();
  }

  shared_count(shared_count&& r): pi_(r.pi_) {
    r.pi_ = 0;
  }

  explicit shared_count(weak_count const & r) {
    if(pi_ != 0 && !pi_->add_ref_lock()) {
      pi_ = 0;
    }
  }

  shared_count & operator= (shared_count const & r) {
    sp_counted_base * tmp = r.pi_;

    if( tmp != pi_ ) {
      if( tmp != 0 ) tmp->add_ref_copy();
      if( pi_ != 0 ) pi_->release();
      pi_ = tmp;
    }

    return *this;
  }
};

class weak_count {
 private:
  sp_counted_base * pi_;

 public:
    weak_count(): pi_(0) { }
    weak_count(shared_count const & r): pi_(r.pi_) {
      if(pi_ != 0) pi_->weak_add_ref();
    }

    weak_count(weak_count const & r): pi_(r.pi_) {
      if(pi_ != 0) pi_->weak_add_ref();
    }

    weak_count(weak_count && r): pi_(r.pi_) {
      r.pi_ = 0;
    }

    ~weak_count() {
      if(pi_ != 0) pi_->weak_release();
    }

    weak_count & operator= (shared_count const & r) {
      sp_counted_base * tmp = r.pi_;

      if(tmp != pi_) {
        if(tmp != 0) tmp->weak_add_ref();
        if(pi_ != 0) pi_->weak_release();
        pi_ = tmp;
      }
      return *this;
    }

    weak_count & operator= (weak_count const & r) {
      sp_counted_base * tmp = r.pi_;

      if( tmp != pi_ ) {
            if(tmp != 0) tmp->weak_add_ref();
            if(pi_ != 0) pi_->weak_release();
            pi_ = tmp;
        }

        return *this;
    }

    bool empty() const {
      return pi_ == 0;
    }
};
