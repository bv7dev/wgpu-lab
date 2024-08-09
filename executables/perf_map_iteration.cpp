#include <cassert>
#include <chrono>
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>

int main() {
  std::vector<uint64_t> vkey{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
  std::vector<uint64_t> vval{12, 18, 11, 10, 13, 14, 20, 42, 9, 2, 8, 5, 17, 14, 88, 7};

  assert(vkey.size() == vval.size());

  std::unordered_map<uint64_t, uint64_t> umap;
  std::map<uint64_t, uint64_t> omap;

  for (int i = 0; i < vkey.size(); ++i) {
    umap[vkey[i]] = vval[i];
    omap[vkey[i]] = vval[i];
  }

  // Performance testing ---------------------------------------------
  {
    const int ROUNDS = 1'000'000'000;
    uint64_t res = 0;

    // unordered map iteration performance
    auto start = std::chrono::steady_clock::now();
    for (int r = 0; r < ROUNDS; ++r) {
      for (auto [k, v] : umap) {
        res += k + v;
      }
    }
    auto end = std::chrono::steady_clock::now();
    std::cout << "umap: " << (end - start) / ROUNDS << ", res: " << res << std::endl;

    // ordered map iteration performance
    res = 0;
    start = std::chrono::steady_clock::now();
    for (int r = 0; r < ROUNDS; ++r) {
      for (auto [k, v] : omap) {
        res += k + v;
      }
    }
    end = std::chrono::steady_clock::now();
    std::cout << "omap: " << (end - start) / ROUNDS << ", res: " << res << std::endl;

    // two vectors iteration performance
    res = 0;
    start = std::chrono::steady_clock::now();
    for (int r = 0; r < ROUNDS; ++r) {
      for (int i = 0; i < vkey.size(); ++i) {
        res += vkey[i] + vval[i];
      }
    }
    end = std::chrono::steady_clock::now();
    std::cout << "vecs: " << (end - start) / ROUNDS << ", res: " << res << std::endl;
  }
}
