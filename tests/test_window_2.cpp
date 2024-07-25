#include <lab.h>

int main() {
  lab::Window first_window{640, 400, "My Window"};
  lab::Window second_window{300, 200, "Test"};

  lab::Webgpu webgpu{};
  webgpu.create_pipeline();

  lab::Surface first_surface{first_window, webgpu};
  lab::Surface second_surface{second_window, webgpu};

  while (lab::tick()) {
    if (first_window.is_open()) first_surface.render_frame();
    if (second_window.is_open()) second_surface.render_frame();
    lab::sleep(16ms);
  }

  return 0;
}
