#include <lab>

int main() {
  lab::Window window{640, 400, "My Window"};

  lab::Webgpu webgpu{"My instance"};

  lab::Shader shader{"test.wgsl"};
  shader.transfer(webgpu.device);
  webgpu.create_pipeline(shader.module);

  // lab::transfer(shader, webgpu);
  // lab::transfer(buffer, webgpu); ... overload for data transfers in general ?

  lab::Surface surface{window, webgpu};

  // lab::Surface surface = lab::create_surface(window, webgpu); --> requires move

  while (lab::tick()) {
    webgpu.render_frame(surface.wgpu_surface);

    // lab::render(surface);

    lab::sleep(16ms);
  }

  return 0;
}
