#include <lab>

// todo: it seems like my webgpu error callback is not working, so i don't get any error messages

int main() {
  lab::Webgpu webgpu("My Instance");
  lab::Shader shader("My Shader", "shaders/vbuf.wgsl");

  lab::Window window("A Window with tutorial code", 640, 400);
  lab::Surface surface(window, webgpu);

  std::vector<float> vertexData = {-0.5,   -0.5, +0.5,   -0.5, +0.0,   +0.5,
                                   -0.55f, -0.5, -0.05f, +0.5, -0.55f, +0.5};

  uint32_t vertexCount = static_cast<uint32_t>(vertexData.size() / 2);

  lab::ReadableBuffer<float> buffer("My vertex buffer", vertexData, webgpu);

  lab::Pipeline mypip(buffer, shader, webgpu);
  mypip.init();

  window.set_resize_callback(
      [&surface](int width, int height) { surface.reconfigure(width, height); });

  while (lab::tick()) {
    mypip.render_frame(surface, {vertexCount, 1});
    lab::sleep(50ms);
  }
}
