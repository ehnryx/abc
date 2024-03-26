// Usage:  treap::node<Tag1, Tag2, ...>* root;
// auto [t0, t1, ..., tn] treap::split<Tag>(root, k1, k2, ..., kn);
// t = treap::merge(t0, t1, ..., tn);
// lookup by splitting
// TESTED ON
// - https://ccpc24.kattis.com/problems/conquestofthe7seas
//     with_parent, ordering, sum_type, onion
// - https://codeforces.com/gym/104941/problem/F
//   ordering
// - https://codeforces.com/gym/102787/problem/A
//   count
#pragma once

#include <cstdlib>
#include <functional>
#include <tuple>
#include <utility>

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

template <class T>
struct with_parent {
  T* p = nullptr;
};
template <>
struct with_parent<void> {};

template <bool has_parent, class... Ts>
struct node : with_parent<std::conditional_t<has_parent, node<has_parent, Ts...>, void>> {
  // TODO assert Ts... are unique?
  template <size_t I>
  using element_t = std::tuple_element_t<I, std::tuple<Ts...>>;
  constexpr static size_t size_v = sizeof...(Ts);
  constexpr static bool has_parent_v = has_parent;
  node* l = nullptr;
  node* r = nullptr;
  int y = std::rand();
  std::tuple<Ts...> t = {};
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
  ~node() {
    delete l;
    delete r;
  }
};

template <class T, class Node>
T* get_ptr(Node* p) {
  return p ? &p->template get<T>() : nullptr;
}

template <size_t I, class Node>
auto* get_ptr(Node* p) {
  return p ? &p->template get<I>() : nullptr;
}

// merge operation
template <class Node>
Node* pull(Node* p) {
  assert(p != nullptr);
  for_sequence(std::make_index_sequence<Node::size_v>{}, [&](auto I) {
    using T = typename Node::element_t<I>;
    if constexpr (has_pull_v<T>) {
      get<I>(p->t).pull(get_ptr<I>(p->l), get_ptr<I>(p->r));
    }
  });
  return p;
}

/* lazy operations */
template <class Node>
void push(Node* p) {
  assert(p != nullptr);
  for_sequence(std::make_index_sequence<Node::size_v>{}, [&](auto I) {
    using T = typename Node::element_t<I>;
    if constexpr (has_push_v<T>) {
      get<I>(p->t).push(get_ptr<I>(p->l), get_ptr<I>(p->r));
    }
  });
}

// returns {L, R : elems >= k}  (lower_bound)
//         {L, R : elems >  k}  (upper_bound)
template <class T, bool upper_bound = false, class Node>
std::tuple<Node*, Node*> split(Node* p, typename T::comparable_type k) {
  static Node* t;
  if (!p) return {nullptr, nullptr};
  push(p);
  if constexpr (requires(T a) {
                  { T::split_remove(k, a, &a) };
                }) {
    if (upper_bound ? T::compare(get_ptr<T>(p->l), k) > 0
                    : T::compare(get_ptr<T>(p->l), k) >= 0) {
      std::tie(t, p->l) = split<T, upper_bound>(p->l, k);
      if constexpr (Node::has_parent_v) {
        if (t) t->p = nullptr;
        if (p->l) p->l->p = p;
      }
      return {t, pull(p)};
    }
    T::split_remove(k, std::get<T>(p->t), get_ptr<T>(p->l));
  } else {
    if (upper_bound ? T::compare(&std::get<T>(p->t), k) > 0
                    : T::compare(&std::get<T>(p->t), k) >= 0) {
      std::tie(t, p->l) = split<T, upper_bound>(p->l, k);
      if constexpr (Node::has_parent_v) {
        if (t) t->p = nullptr;
        if (p->l) p->l->p = p;
      }
      return {t, pull(p)};
    }
  }
  std::tie(p->r, t) = split<T, upper_bound>(p->r, k);
  if constexpr (Node::has_parent_v) {
    if (t) t->p = nullptr;
    if (p->r) p->r->p = p;
  }
  return {pull(p), t};
}

template <class Node>
Node* merge(Node* l, Node* r) {
  if (!l) return r;
  if (!r) return l;
  if (l->y > r->y) {
    push(l);
    l->r = merge(l->r, r);
    if constexpr (Node::has_parent_v) {
      if (l->r) l->r->p = l;
      l->p = nullptr;
    }
    return pull(l);
  } else {
    push(r);
    r->l = merge(l, r->l);
    if constexpr (Node::has_parent_v) {
      if (r->l) r->l->p = r;
      r->p = nullptr;
    }
    return pull(r);
  }
}

// merge with comparator
template <class T, class Node>
Node* onion(Node* t1, Node* t2) {
  if (!t1) return t2;
  if (!t2) return t1;
  if (t1->y < t2->y) swap(t1, t2);
  auto [l, r] = split<T>(t2, t1->template get<T>().v);
  auto* t1l = std::exchange(t1->l, nullptr);
  auto* t1r = std::exchange(t1->r, nullptr);
  if (t1l) t1l->p = nullptr;
  if (t1r) t1r->p = nullptr;
  return merge(onion<T>(t1l, l), t1, onion<T>(t1r, r));
}

template <typename Node>
Node* leftmost(Node* u) {
  if (u != nullptr) {
    while (u->l != nullptr) {
      push(u);
      u = u->l;
    }
  }
  return u;
}

template <typename Node>
Node* rightmost(Node* u) {
  if (u != nullptr) {
    while (u->r != nullptr) {
      push(u);
      u = u->r;
    }
  }
  return u;
}

template <class F, class Node>
void visit(Node* p, const F& f) {
  if (!p) return;
  push(p);
  visit(p->l, f);
  f(p);
  visit(p->r, f);
}

// Syntax Sugar
/* optional, multi merge/split */
template <class A, class... Args>
A merge(A l, A r, Args... rest) {
  return merge(merge(l, r), rest...);
}

namespace detail {
template <size_t I, class T, class Ret>
void multi_split_helper(Ret&) {}
template <size_t I, class T, class Ret, class K, class... Args>
void multi_split_helper(Ret& ret, K k, Args... ks) {
  if constexpr (I == sizeof...(ks) + 2) return;
  multi_split_helper<I + 1, T>(ret, ks...);
  std::tie(ret[I], ret[I + 1]) = split<T>(ret[I + 1], k);
}
}  // namespace detail

// [0, k), [k, k2), [k2, k3), ...
template <class T, class Node, class... Args>
std::array<Node*, sizeof...(Args) + 1> split_multi(Node* p, Args... ks) {
  static_assert(std::conjunction_v<std::is_same<typename T::comparable_type, Args>...>);
  std::array<Node*, sizeof...(Args) + 1> ret{};
  ret.back() = p;
  detail::multi_split_helper<0, T>(ret, ks...);
  return ret;
}

template <class T, class Node>
std::tuple<Node*, Node*> split_lower_bound(Node* p, typename T::comparable_type k) {
  return split<T, false>(p, k);
}

template <class T, class Node>
std::tuple<Node*, Node*> split_upper_bound(Node* p, typename T::comparable_type k) {
  return split<T, true>(p, k);
}

template <class T, class Node>
std::array<Node*, 3> split_equal_range(Node* p, typename T::comparable_type k) {
  std::array<Node*, 3> ret;
  std::tie(p, ret[2]) = split_upper_bound<T>(p, k);
  std::tie(ret[0], ret[1]) = split_lower_bound<T>(p, k);
  return ret;
}

// for ordered_set operations
// split k => [left tree < k, right tree >= k]
template <class T = int, class Compare = std::less<T>>
struct ordering {
  using value_type = T;
  using comparable_type = T;
  T v;
  ordering& operator=(const T& t) {
    v = t;
    return *this;
  }
  static auto compare(ordering* p, comparable_type k) {
    assert(p);
    return p->v <=> k;
  }
};

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

template <class Value = int, class Sum = long long>
struct sum_type {
  Value val = 0;
  Sum sum = 0;

  void pull(sum_type* l, sum_type* r) {
    sum = val;
    if (l) sum += l->sum;
    if (r) sum += r->sum;
  }
};

};  // namespace treap

// try problem K: N^2 unordered pairs of vertices in tree => (x,lca(x,y),y)
// query: what is kth largest tuple
// lca is on path from root => x including x, number of times node occurs as lca is size of subtree

// try problem K: N^2 unordered pairs of vertices in tree => (x,lca(x,y),y)
// query: what is kth largest tuple
// lca is on path from root => x including x, number of times node occurs as lca is size of subtree
