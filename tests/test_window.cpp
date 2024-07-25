#include <lab>

int main() {
  lab::Window window{640, 400, "My Window"};
  lab::Webgpu webgpu{"My instance"};
  lab::Shader shader{"shaders/test.wgsl"};
  lab::Surface surface{window, webgpu};

  shader.transfer(webgpu.device);
  webgpu.create_pipeline(shader.module);

  while (lab::tick()) {
    webgpu.render_frame(surface.wgpu_surface);
    lab::sleep(16ms);
  }

  return 0;
}
