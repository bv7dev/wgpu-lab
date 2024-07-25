#include <lab>

int main() {
  lab::Webgpu webgpu{"My Instance"};
  lab::Window window{"My Window", 640, 400};
  lab::Shader shader{"My Shader", "shaders/test.wgsl"};

  lab::Surface surface{window, webgpu};
  lab::create_pipeline(webgpu, shader);

  while (lab::tick()) {
    lab::render_frame(surface);
    lab::sleep(16ms);
  }

  return 0;
}
