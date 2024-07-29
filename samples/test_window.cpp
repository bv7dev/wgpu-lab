#include <lab>

#include <GLFW/glfw3.h>

int main() {
  using namespace lab;

  Webgpu webgpu{"My Instance"};
  Window window{"My Window", 640, 400};
  Shader shader{"My Shader", "shaders/test1.wgsl"};

  Surface surface{window, webgpu};
  Pipeline pipeline{shader, webgpu};

  window.set_resize_callback([&surface](int width, int height) {
    surface.reconfigure(width, height);
  });

  window.set_key_callback([&window](const Window::KeyEvent& event) {
    if (event.key == GLFW_KEY_SPACE && event.action == GLFW_PRESS) {
      window.clear_resize_callback();
    }
  });

  while (tick()) {
    pipeline.render_frame(surface);
    sleep(16ms);
  }
}
