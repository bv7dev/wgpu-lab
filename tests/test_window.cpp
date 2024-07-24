#include <lab.h>

#include <chrono>
#include <iostream>
#include <thread>

int main() {
  lab::Window first_window{640, 400, "My Window"};
  lab::Window second_window{300, 200, "Test"};

  lab::Webgpu wgpu;

  first_window.init_surface(wgpu);
  second_window.init_surface(wgpu);

  wgpu.init(first_window.surface);
  wgpu.create_pipeline();

  while (lab::tick()) {
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(16ms);

    if (first_window.is_open()) {
      wgpu.configure_surface(first_window.surface, 640, 400);
      wgpu.render_frame(first_window.surface);
    }

    if (second_window.is_open()) {
      wgpu.configure_surface(second_window.surface, 200, 200);
      wgpu.render_frame(second_window.surface);
    }
  }

  return 0;
}
