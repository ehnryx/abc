#include "data_structures/segment_tree.h"
#include "todo/main.h"

struct node {
  ll up = 0;
  ll down = 0;
  node() = default;
  void put(ll v) { up = down = v; }
  ll get(segment_accumulate_t<ll> acc) const { return max(*acc, up); }
  void pull(node const& l, node const& r) { up = max(l.up, r.up); }
  segment_accumulate_t<ll> accumulate(segment_accumulate_t<ll> acc) const {
    return segment_accumulate_t{max(*acc, down)};
  }
  static ll merge(ll l, ll r, ...) { return max(l, r); }
};

SOLVE() {
  int n;
  cin >> n;
  auto st = segment_tree<node, segment_tree_traits::SPARSE>((ll)2e9 + 1);
  for (int i = 0; i < n; i++) {
    int x, s;
    cin >> x >> s;
    auto bottom = st.query_range(x, x + s - 1, segment_accumulate_t<ll>{0});
    st.update_range(x, x + s - 1, bottom + s);
    cout << st.root().up << nl;
  }
}
