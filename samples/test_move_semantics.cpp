#include <cassert>
#include <iostream>
#include <vector>

struct VMap {
  std::string label;
  std::vector<int> data;

  VMap(const std::string& label, size_t cap) : label{std::move(label)} { data.reserve(cap); }

  VMap(const VMap& other) : data{other.data} {
    label = other.label + " copy";
    std::cout << "\n\ncopy performed\n\n";
  }
  VMap& operator=(const VMap& other) {
    data = other.data;
    return *this;
  }

  VMap(VMap&& other) : data{std::move(other.data)}, label{std::move(other.label)} {
    std::cout << "\n\nmove performed\n\n";
  }

  void push(int v) {
    assert(data.size() < data.capacity());
    data.push_back(v);
  }
  void print() {
    std::cout << label << " -> [";
    for (const int& v : data) {
      std::cout << v;
      if (&v != &data.back()) {
        std::cout << ", ";
      }
    }
    std::cout << "]\n";
  }
};

int main() {
  VMap vmap_a("A", 5);
  VMap vmap_b("B", 5);

  vmap_a.push(1);
  vmap_a.push(3);
  vmap_a.push(2);

  vmap_b.push(7);
  vmap_b.push(9);
  vmap_b.push(8);

  vmap_a.print();
  vmap_b.print();

  VMap cpy = vmap_a;
  vmap_a.data[0] = 5;

  cpy.print();
  vmap_a.print();

  VMap new_owner = std::move(cpy);
  new_owner.print();
  cpy.print();

  return 0;
}
