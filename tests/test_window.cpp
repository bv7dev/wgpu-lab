#include <lab.h>

int main() {
  lab::Window window{640, 400, "My Window"};

  lab::Webgpu webgpu{};
  webgpu.create_pipeline();

  lab::Surface surface{window, webgpu};

  while (lab::tick()) {
    surface.render_frame();
    lab::sleep(16ms);
  }

  return 0;
}
