#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

int main() {
  using namespace std;
  using namespace std::chrono_literals;

  int x = 0;

  auto t1 = make_unique<thread>([&]() {
    for (int i = 0; i < 20; ++i) {
      x += i;
      std::cout << "thread t1: tick " << i << std::endl;
      this_thread::sleep_for(1s);
    }
  });

  for (int i = 0; i < 20; ++i) {
    std::cout << "main thread: tick " << i << std::endl;
    this_thread::sleep_for(1s);
  }

  t1->join();
}
