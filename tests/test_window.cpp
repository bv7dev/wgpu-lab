#include "lab_webgpu.h"
#include "lab_window.h"

#include <chrono>
#include <iostream>
#include <thread>

int main() {
  lab::Window first_window{640, 400, "My Window"};
  lab::Window second_window{300, 200, "Test"};

  lab::create_webgpu_instance(first_window);

  second_window.set_key_callback([&](int key, int scancode, int action, int mod) {
    if (key == 32 && action == 0 && mod == 0x0001) {
      first_window.~Window();
      std::cout << "test " << scancode << std::endl;
    }
  });

  while (lab::tick()) {
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(16ms);
  }

  return 0;
}
