template <class T> struct static_alloc_ptr {
  static_alloc_ptr(size_t i=0) : idx(i) { }
  T &operator*() const { return data[idx]; }
  T &at() const { return data[idx]; }
  T *operator->() const { return &data[idx]; }
  auto operator<=>(const static_alloc_ptr &o) const { return idx <=> o.idx; }
  operator bool() const { return idx != 0; }
  static void init(T* d) { data = d; }

private:
  inline static T* data = nullptr;
  friend std::ostream &operator<<(std::ostream &os, const static_alloc_ptr &p) {
    return os << "static_alloc_ptr<" << typeid(T).name() << ">[" << p.idx
              << "]";
  }
  size_t idx;
};
