#include <lab>
using namespace lab;

int main() {
  Webgpu webgpu("My Instance");
  Shader shader("My Shader", "shaders/test1.wgsl");
  Window window("press space to say hello", 900, 400);

  Surface surface(window, webgpu);
  Pipeline pipeline(shader, webgpu);

  window.set_resize_callback(
      [&surface](int width, int height) { surface.reconfigure(width, height); });

  window.set_key_callback([&window](auto& event) {
    if (event == KeyEvent{KeyCode::space, InputAction::release}) {
      window.set_title("world!");
    }
  });

  while (tick()) {
    pipeline.render_frame(surface);
    lab::sleep(50ms); // saves my laptop from overheating
  }
}
