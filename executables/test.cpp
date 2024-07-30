#include <lab>

enum class test { A = 2, B = 3 };

int main() {
  test x = test::A;

  if (test::A == x) std::cout << "working";

  int b = static_cast<int>(x);
  return b;
}
