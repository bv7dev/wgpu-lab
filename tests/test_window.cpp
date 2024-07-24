#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu/webgpu.hpp>

#include <lab.h>

#include <chrono>
#include <iostream>
#include <thread>

int main() {
  lab::Window first_window{640, 400, "My Window"};
  lab::Window second_window{300, 200, "Test"};

  lab::Webgpu wgpu;
  lab::link(first_window, wgpu);

  // lab::Webgpu wgpu2;
  // lab::link(second_window, wgpu2);

  lab::prp(second_window, wgpu);

  // bool wgpu2_render = false;

  first_window.set_key_callback([&](int key, int, int action, int mod) {
    if (key == 32 && action == 0 && mod == 0x0001) {
      // wgpu2_render = !wgpu2_render;
    }
  });

  second_window.set_key_callback([&](int key, int scancode, int action, int mod) {
    if (key == 32 && action == 0 && mod == 0x0001) {
      first_window.~Window();
      std::cout << "test " << scancode << std::endl;
    }
  });

  while (lab::tick()) {
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(16ms);

    // if (wgpu2_render) wgpu2.render_frame();
    wgpu.render_frame();

    lab::swt(second_window, wgpu);

    wgpu.render_frame();

    lab::bkk(first_window, wgpu);
  }

  return 0;
}
