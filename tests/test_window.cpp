#include <lab>

using namespace lab;

int main() {
  Webgpu webgpu{"My Instance"};
  Window window{"My Window", 640, 400};
  Shader shader{"My Shader", "shaders/test1.wgsl"};

  Surface surface{window, webgpu};
  Pipeline pipeline{shader, webgpu};

  window.set_resize_callback([&surface](int width, int height) {
    surface.reconfigure(width, height);
  });

  while (tick()) {
    pipeline.render_frame(surface);
    sleep(16ms);
  }
}
