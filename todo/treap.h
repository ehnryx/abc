// Usage:  treap::node<Tag1, Tag2, ...>* root;
// auto [t0, t1, ..., tn] treap::split<Tag>(root, k1, k2, ..., kn);
// t = treap::merge(t0, t1, ..., tn);
// lookup by splitting
namespace treap {
// tuple index helper
template <typename T, T... S, typename F>
constexpr void for_sequence(std::integer_sequence<T, S...>, F&& f) {
  (void(f(std::integral_constant<T, S>{})), ...);
}

template <typename T>
concept has_pull_v = requires(T a, T* p) {
                       { a.pull(p, p) };
                     };

template <typename T>
concept has_push_v = requires(T a, T* p) {
                       { a.push(p, p) };
                     };

template <class... Ts>
struct node {
  // TODO assert Ts... are unique?
  node* l = nullptr;
  node* r = nullptr;
  int y = rand();
  tuple<Ts...> t = {};
  template <class T>
  T& get() {
    return std::get<T>(t);
  }
  template <class T>
  const T& get() const {
    return std::get<T>(t);
  }
  ~node() {
    delete l;
    delete r;
  }
};

template <class T, class... Ts>
T* get_ptr(node<Ts...>* p) {
  if (!p) return nullptr;
  return &p->template get<T>();
}

// merge operation
template <class... Ts>
node<Ts...>* pull(node<Ts...>* p) {
  assert(p != nullptr);
  for_sequence(make_index_sequence<sizeof...(Ts)>{}, [&](auto I) {
    using T = std::tuple_element_t<I, tuple<Ts...>>;
    if constexpr (has_pull_v<T>) {
      get<I>(p->t).pull(get_ptr<T>(p->l), get_ptr<T>(p->r));
    }
  });
  return p;
}

/* lazy operations */
template <class... Ts>
void push(node<Ts...>* p) {
  assert(p != nullptr);
  for_sequence(make_index_sequence<sizeof...(Ts)>{}, [&](auto I) {
    using T = std::tuple_element_t<I, tuple<Ts...>>;
    if constexpr (has_push_v<T>) {
      get<I>(p->t).push(get_ptr<T>(p->l), get_ptr<T>(p->r));
    }
  });
}
// returns [0, k), [k, end)
template <class T, class Node>
tuple<Node*, Node*> split(Node* p, typename T::comparable_type k) {
  static Node* t;
  if (!p) return {nullptr, nullptr};
  push(p);

  if constexpr (requires(T a) {
                  { T::split_remove(k, a, &a) };
                }) {
    if (T::at_least(get_ptr<T>(p->l), k)) {
      tie(t, p->l) = split<T>(p->l, k);
      return {t, pull(p)};
    }
    T::split_remove(k, std::get<T>(p->t), get_ptr<T>(p->l));
  } else {
    if (T::at_least(&std::get<T>(p->t), k)) {
      tie(t, p->l) = split<T>(p->l, k);
      return {t, pull(p)};
    }
  }
  tie(p->r, t) = split<T>(p->r, k);
  return {pull(p), t};
}

template <class Node>
Node* merge(Node* l, Node* r) {
  if (!l) return r;
  if (!r) return l;
  if (l->y > r->y) {
    push(l);
    l->r = merge(l->r, r);
    return pull(l);
  } else {
    push(r);
    r->l = merge(l, r->l);
    return pull(r);
  }
}
template <class F, class... Ts>
void visit(node<Ts...>* p, const F& f) {
  if (!p) return;
  push(p);
  visit(p->l, f);
  f(p);
  visit(p->r, f);
}
/* optional, multi merge/split */
template <class A, class... Args>
A merge(A l, A r, Args... rest) {
  return merge(merge(l, r), rest...);
}
// [0, k), [k, k2), [k3, k4), ...
template <class T, class Node, class... Args>
auto split(Node* p, typename T::comparable_type k, Args... rest) {
  auto others = split<T>(p, rest...);
  tie(p, std::get<0>(others)) = split<T>(std::get<0>(others), k);
  return std::tuple_cat(std::make_tuple(p), others);
}

// for ordered_set operations
// split k => [left tree < k, right tree >= k]
// tested on https://codeforces.com/gym/104941/problem/F
template <class T = int, class Compare = less<T>>
struct ordering {
  using value_type = T;
  using comparable_type = T;
  T v;
  ordering& operator=(const T& t) {
    v = t;
    return SELF;
  }
  static bool at_least(ordering* p, comparable_type k) {
    assert(p);
    return p->v >= k;
  }
};

// for segtree like operations
// split k => [left size k tree, right tree]
// tested on https://codeforces.com/gym/102787/problem/A
struct count {
  using comparable_type = int;
  int cnt = 1;

  static bool at_least(count* p, comparable_type k) {
    if (!p) return 0 >= k;
    return p->cnt >= k;
  }

  void pull(count* l, count* r) {
    cnt = 1;
    if (l) cnt += l->cnt;
    if (r) cnt += r->cnt;
  }

  static void split_remove(comparable_type& k, const count& /* root */, count* l) {
    k -= 1;  // root
    if (l) k -= l->cnt;
  }
};

};  // namespace treap

// try problem K: N^2 unordered pairs of vertices in tree => (x,lca(x,y),y)
// query: what is kth largest tuple
// lca is on path from root => x including x, number of times node occurs as lca is size of subtree
