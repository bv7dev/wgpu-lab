#include <lab>
using namespace lab;

int main() {
  Webgpu webgpu("My Instance");
  Shader shader("My Shader", "shaders/test1.wgsl");
  Window window("A Window and a Triangle", 640, 400);

  Surface surface(window, webgpu);
  Pipeline pipeline(shader, webgpu);
  pipeline.finalize();

  while (tick()) {
    pipeline.render_frame(surface, {3, 1});
  }
}
