#include "lab_window.h"

#include <iostream>

int main() {
  lab::Window first_window{640, 400, "My window"};
  lab::Window second_window{300, 200, "Test"};

  second_window.set_callback([&](int key, int scancode, int action, int mod) {
    if (key == 32 && action == 0 && mod == 0x0001) {
      first_window.~Window();
      std::cout << "test " << scancode << std::endl;
    }
  });

  while (lab::tick()) {
    // main loop
  }

  return 0;
}
