// Usage:  treap::node<Tag1, Tag2, ...>* root;
// auto [t0, t1, ..., tn] treap::split<Tag>(root, k1, k2, ..., kn);
// t = treap::merge(t0, t1, ..., tn);
// lookup by splitting
// TESTED ON
// - https://ccpc24.kattis.com/problems/conquestofthe7seas
//     node_base, ordering, sum_type, onion
// - https://codeforces.com/gym/104941/problem/F
//   ordering
// - https://codeforces.com/gym/102787/problem/A
//   count
//
%:include "utility/static_alloc_ptr.h"

namespace {
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

template <class T>
struct with_parent {
  T p=0;
};
template <>
struct with_parent<void> {};
}

template <bool has_parent, size_t N, class... Ts>
struct treap {

template <size_t I>
using element_t = std::tuple_element_t<I, tuple<Ts...>>;
constexpr static size_t size_v = sizeof...(Ts);
constexpr static bool has_parent_v = has_parent;


struct node : with_parent<std::conditional_t<has_parent,static_alloc_ptr<node>,void>> {
  static_alloc_ptr<node> l=0;
  static_alloc_ptr<node> r=0;
  tuple<Ts...> t = {};
  template <class T>
  T& get() {
    return std::get<T>(t);
  }
  template <class T>
  const T& get() const {
    return std::get<T>(t);
  }
  template <size_t I>
  auto& get() {
    return std::get<I>(t);
  }
  template <size_t I>
  const auto& get() const {
    return std::get<I>(t);
  }
};

using ptr = static_alloc_ptr<node>;
inline static std::array<node,N> data_;
// allocation
inline static std::array<int,N> alloc_;
static void init() {
  static bool once = [](){
    ptr::init(data_.begin());
    std::iota(alloc_.begin()+1, alloc_.end(),1);
    return true;
  }();
  alloc_[0] = 1;
  static auto rng = std::minstd_rand();
  shuffle(alloc_.begin()+1, alloc_.end(), rng);
}
static ptr alloc() {
  return alloc_[alloc_[0]++];
}


template <class T>
static T* get_ptr(ptr p) {
  return p ? &p->template get<T>() : nullptr;
}

template <size_t I>
static auto* get_ptr(ptr p) {
  return p ? &p->template get<I>() : nullptr;
}

// merge operation
static ptr pull(ptr p) {
  assert(p != 0);
  for_sequence(make_index_sequence<size_v>{}, [&](auto I) {
    using T = element_t<I>;
    if constexpr (has_pull_v<T>) {
      p->template get<I>().pull(get_ptr<I>(p->l), get_ptr<I>(p->r));
    }
  });
  return p;
}

/* lazy operations */
static void push(ptr p) {
  assert(p != 0);
  for_sequence(make_index_sequence<size_v>{}, [&](auto I) {
    using T = element_t<I>;
    if constexpr (has_push_v<T>) {
      p->template get<I>().push(get_ptr<I>(p->l), get_ptr<I>(p->r));
    }
  });
}

// returns {L, R : elems >= k}  (lower_bound)
//         {L, R : elems >  k}  (upper_bound)
template <class T, bool upper_bound = false>
static tuple<ptr,ptr> split(ptr p, typename T::comparable_type k) {
  static ptr t;
  if (!p) return {0, 0};
  push(p);
  if constexpr (requires(T a) {
                  { T::split_remove(k, a, &a) };
                }) {
    if (upper_bound ? T::compare(get_ptr<T>(p->l), k) > 0
                    : T::compare(get_ptr<T>(p->l), k) >= 0) {
      tie(t, p->l) = split<T, upper_bound>(p->l, k);
      if constexpr (has_parent_v) {
        if (t) t->p = nullptr;
        if (p->l) p->l->p = p;
      }
      return {t, pull(p)};
    }
    T::split_remove(k, std::get<T>(p->t), get_ptr<T>(p->l));
  } else {
    if (upper_bound ? T::compare(&std::get<T>(p->t), k) > 0
                    : T::compare(&std::get<T>(p->t), k) >= 0) {
      tie(t, p->l) = split<T, upper_bound>(p->l, k);
      if constexpr (has_parent_v) {
        if (t) t->p = nullptr;
        if (p->l) p->l->p = p;
      }
      return {t, pull(p)};
    }
  }
  tie(p->r, t) = split<T, upper_bound>(p->r, k);
  if constexpr (has_parent_v) {
    if (t) t->p = nullptr;
    if (p->r) p->r->p = p;
  }
  return {pull(p), t};
}

static ptr merge(ptr l, ptr r) {
  if (!l) return r;
  if (!r) return l;
  if (l > r) {
    push(l);
    l->r = merge(l->r,r);
    if constexpr (has_parent_v) {
      if (l->r) l->r->p = l;
      l->p = 0;
    }
    return pull(l);
  } else {
    push(r);
    r->l = merge(l, r->l);
    if constexpr (has_parent_v) {
      if (r->l) r->l->p = r;
      r->p = 0;
    }
    return pull(r);
  }
}

// merge with ordering comparator
template <class T>
static ptr onion(ptr t1, ptr t2) {
  if (!t1) return t2;
  if (!t2) return t1;
  if (t1 < t2) swap(t1, t2);
  auto [l, r] = split<T>(t2, t1->template get<T>().v);
  ptr t1l = std::exchange(t1->l, 0);
  ptr t1r = std::exchange(t1->r, 0);
  if (t1l) t1l->p = 0;
  if (t1r) t1r->p = 0;
  return merge(onion<T>(t1l, l), t1, onion<T>(t1r, r));
}

template <class F>
static void visit(ptr p, const F& f) {
  if (!p) return;
  push(p);
  visit(p->l, f);
  f(p);
  visit(p->r, f);
}

// Syntax Sugar
/* optional, multi merge/split */
template <class... Args>
static ptr merge(ptr l, ptr r, Args... rest) {
  return merge(merge(l, r), rest...);
}

template <size_t I, class T, class Ret>
static void multi_split_helper(Ret&) {}
template <size_t I, class T, class Ret, class K, class... Args>
static void multi_split_helper(Ret& ret, K k, Args... ks) {
  if constexpr (I == sizeof...(ks) + 2) return;
  multi_split_helper<I + 1, T>(ret, ks...);
  tie(ret[I], ret[I + 1]) = split<T>(ret[I + 1], k);
}

// [0, k), [k, k2), [k2, k3), ...
template <class T, class... Args>
static std::array<ptr, sizeof...(Args) + 1> split_multi(ptr p, Args... ks) {
  //static_assert(std::conjunction_v<std::is_same<typename T::comparable_type, Args>...>);
  std::array<ptr, sizeof...(Args) + 1> ret;
  ret.back() = p;
  multi_split_helper<0, T>(ret, ks...);
  return ret;
}

template <class T>
static std::tuple<ptr, ptr> split_lower_bound(ptr p, typename T::comparable_type k) {
  return split<T, false>(p, k);
}

template <class T>
std::tuple<ptr, ptr> split_upper_bound(ptr p, typename T::comparable_type k) {
  return split<T, true>(p, k);
}

template <class T>
std::array<ptr, 3> split_equal_range(ptr p, typename T::comparable_type k) {
  std::array<ptr, 3> ret;
  tie(p, ret[2]) = split_upper_bound<T>(p, k);
  tie(ret[0], ret[1]) = split_lower_bound<T>(p, k);
  return ret;
}

// for ordered_set operations
// split k => [left tree < k, right tree >= k]
template <class T = int, class Compare = less<T>>
struct ordering {
  using value_type = T;
  using comparable_type = T;
  T v;
  ordering& operator=(const T& t) {
    v = t;
    return SELF;
  }
  static auto compare(ordering* p, comparable_type k) {
    assert(p);
    return p->v <=> k;
  }
};

};

namespace data {
// for segtree like operations
// split k => [left size k tree, right tree]
struct count {
  using comparable_type = int;
  int cnt = 1;

  static auto compare(count* p, comparable_type k) {
    if (!p) return 0 <=> k;
    return p->cnt <=> k;
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

template <class Value = int, class Sum = ll>
struct sum_type {
  Value val = 0;
  Sum sum = 0;

  void pull(sum_type* l, sum_type* r) {
    sum = val;
    if (l) sum += l->sum;
    if (r) sum += r->sum;
  }
};
}



// try problem K: N^2 unordered pairs of vertices in tree => (x,lca(x,y),y)
// query: what is kth largest tuple
// lca is on path from root => x including x, number of times node occurs as lca is size of subtree

// try problem K: N^2 unordered pairs of vertices in tree => (x,lca(x,y),y)
// query: what is kth largest tuple
// lca is on path from root => x including x, number of times node occurs as lca is size of subtree
