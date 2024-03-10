#include "todo/main.h"
#include "todo/treap2.h"
#include "utility/bump_allocator.h"

SOLVE() {
  int n;
  cin >> n;

  treap_map<
      void, int, treap_traits::ORDER_STATS,
      make_treap_map_allocator<bump_allocator, void, int, treap_traits::ORDER_STATS>>
      arr;
  for (int i = 1; i <= n; i++) {
    arr.emplace_at(i - 1, i);
  }

  for (int i = 0; i < n; i++) {
    int a, b;
    cin >> a >> b;
    a -= 1;
    b -= 1;
    int len = min(b - a, n - b);
    if (len <= 0) continue;
    auto [before, left, mid, right] = arr.split_at(a, a + len, b, b + len);
    arr = std::move(before.append(std::move(right))
                        .append(std::move(mid))
                        .append(std::move(left))
                        .append(std::move(arr)));
  }

  for (int i = 0; i < n; i++) {
    cout << arr.split_at(1).root->value << " ";
  }
  cout << nl;
}
