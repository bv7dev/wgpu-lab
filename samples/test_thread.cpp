#include <chrono>
#include <iostream>
#include <thread>

int main() {
  using namespace std;
  using namespace std::chrono_literals;

  int x = 0;

  thread t1{[&]() {
    for (int i = 0; i < 20; ++i) {
      x += i;
      std::cout << "thread t1: tick " << i << std::endl;
      this_thread::sleep_for(1s);
    }
  }};

  for (int i = 0; i < 20; ++i) {
    std::cout << "main thread: tick " << i << std::endl;
    this_thread::sleep_for(1s);
  }

  t1.join();
}
