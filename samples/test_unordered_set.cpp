#include <iostream>
#include <unordered_set>

struct Link {
  uint16_t a, b;
  operator bool() const noexcept { return a != b; }
  bool selfloop() const noexcept { return a == b; }
  bool operator==(const Link& rhs) const noexcept {
    return (a == rhs.a && b == rhs.b) || (a == rhs.b && b == rhs.a) || (a == b && rhs.a == rhs.b);
  }
};

template<>
struct std::hash<Link> {
  size_t operator()(const Link& lnk) const noexcept {
    return std::hash<uint16_t>{}(lnk.a) ^ std::hash<uint16_t>{}(lnk.b);
  }
};

int main() {
  Link lnk0{0, 0};
  Link lnk1{0, 1};
  Link lnk2{1, 0};
  Link lnk3{1, 1};
  Link lnk4{1, 2};
  Link lnk5{2, 1};
  Link lnk6{2, 2};

  if (!lnk0 && !lnk3 && !lnk6) std::cout << "Link{a, a} is invalid" << std::endl;

  std::cout << std::boolalpha;
  std::cout << "Link{a, a} valid? -> " << (lnk0 || lnk3 || lnk6) << std::endl;
  std::cout << "Link{a, b} valid? -> " << (lnk1 && lnk2 && lnk4 && lnk5) << std::endl;
  std::cout << std::noboolalpha;

  if (lnk0 == lnk3 && lnk0 == lnk6) std::cout << "Link{a, a} equals Link{b, b}" << std::endl;
  if (lnk1 == lnk2 && lnk4 == lnk5) std::cout << "Link{a, b} equals Link{b, a}" << std::endl;
  if (lnk0 == lnk2 && lnk3 == lnk5) std::cout << "Link{a, a} equals Link{a, b}" << std::endl;

  std::unordered_set<Link> links;

  std::cout << "max size: " << links.max_size() << std::endl;

  auto r1 = links.insert(lnk1);
  auto r0 = links.insert(lnk0);
  auto r2 = links.insert(lnk2);
  auto r3 = links.insert(lnk3);
  auto r4 = links.insert(lnk4);
  auto r5 = links.insert(lnk5);
  auto r6 = links.insert(lnk6);

  links.erase(Link{0, 0});

  std::cout << links.bucket_count() << " buckets" << std::endl;
  std::cout << links.size() << " links" << std::endl;

  return 0;
}
