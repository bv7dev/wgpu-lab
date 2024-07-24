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
  lab::link(second_window, wgpu);

  wgpu.init(first_window.surface);
  wgpu.configure_surface(first_window.surface, 640, 400);
  wgpu.create_pipeline();

  while (lab::tick()) {
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(16ms);

    if (first_window.is_open()) {
      lab::swt(first_window, wgpu);
      wgpu.render_frame(first_window.surface);
    }

    if (second_window.is_open()) {
      lab::swt(second_window, wgpu);
      wgpu.render_frame(second_window.surface);
    }
  }

  return 0;
}
