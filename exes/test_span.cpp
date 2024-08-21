#include <iostream>
#include <ranges>

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

  std::cout << "view_a.size() -> " << view_a.size() << std::endl;
  std::cout << "view_a.size_bytes() -> " << view_a.size_bytes() << std::endl;

  for (auto& e : view_a)
    std::cout << e << " ";
  std::cout << std::endl;

  for (auto& e : view_b)
    std::cout << e << " ";
  std::cout << std::endl;

  view_b = {};

  std::cout << view_b.empty() << std::endl;
  std::cout << view_b.size() << std::endl;

  view_a = {view_a.begin(), 8};

  for (auto& e : view_a)
    std::cout << e << " ";
  std::cout << std::endl;
}
