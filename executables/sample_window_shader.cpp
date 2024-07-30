#include <lab>
using namespace lab;

int main() {
  Webgpu webgpu("My Instance");
  Shader shader("My Shader", "shaders/test1.wgsl");
  Window window("A Window and a Triangle", 640, 400);

  Surface surface(window, webgpu);
  Pipeline pipeline(shader, webgpu);

  window.set_resize_callback(
      [&surface](int width, int height) { surface.reconfigure(width, height); });

  while (tick()) {
    pipeline.render_frame(surface);
    lab::sleep(50ms);
  }
}
