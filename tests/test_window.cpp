#include <lab>

int main() {
  lab::Window window{"My Window", 640, 400};
  lab::Webgpu webgpu{"My Instance"};
  lab::Shader shader{"My Shader", "shaders/test.wgsl"};

  lab::create_pipeline(webgpu, shader);
  lab::Surface surface{window, webgpu};

  while (lab::tick()) {
    lab::render_frame(surface);
    lab::sleep(16ms);
  }

  return 0;
}
