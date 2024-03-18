#include "todo/main.h"
#include "todo/splay_forest.h"
#include "utility/bump_allocator.h"

struct node : splay_node_base<node, void, splay_traits::ORDER_STATS> {
  int value;
  node(int v) : value(v) {}
};

SOLVE() {
  int n;
  cin >> n;

  auto st = splay_forest<node, bump_allocator<node>>(n + 1);

  auto root = st.new_node({0}, 0);
  for (int i = 1; i <= n; i++) {
    root = st.append(root, st.new_node({0}, i));
  }

  for (int i = 0; i < n; i++) {
    int a, b;
    cin >> a >> b;
    int len = min(b - a, n - b + 1);
    if (len <= 0) continue;
    decltype(st)::pointer_t left, mid, right, after;
    tie(root, after) = st.split_before(root, b + len, splay_index{});
    tie(root, right) = st.split_before(root, b, splay_index{});
    tie(root, mid) = st.split_before(root, a + len, splay_index{});
    tie(root, left) = st.split_before(root, a, splay_index{});
    root = st.append(root, right);
    if (mid) root = st.append(root, mid);
    root = st.append(root, left);
    if (after) root = st.append(root, after);
  }

  root = st.split_before(root, 1, splay_index{}).second;
  for (int i = 1; i <= n; i++) {
    decltype(st)::pointer_t cur;
    tie(cur, root) = st.split_before(root, 1, splay_index{});
    cout << st.get(cur).value << " ";
  }
  cout << nl;
}
