#include <iostream>
#include <ranges>

// maybe I can use stl span (C++ 20 ranges) to create a view on mapped wgpu
// memory instead of maintaining MappedVRAM class.

int main() {
  constexpr int num_elems = 32;

  int array[num_elems];
  int* begin = new int[num_elems];

  for (int i = 0; i < num_elems; ++i) {
    array[i] = 3 * (i + 1);
    begin[i] = i + 1;
  }

  std::cout << sizeof(begin) << std::endl;
  std::cout << sizeof(array) << std::endl;
  std::cout << std::endl;

  auto view_a = std::span{array, num_elems};
  auto view_b = std::span{begin, num_elems};

  for (auto& e : view_a)
    std::cout << e << " ";
  std::cout << std::endl;

  for (auto& e : view_b)
    std::cout << e << " ";
  std::cout << std::endl;
}
