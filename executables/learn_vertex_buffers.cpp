#include <lab>

#include "tutorial_vertex_buffers.h"

// todo: it seems like my webgpu error callback is not working, so i don't get any error messages

// Assimilate tutorial code !!!

int main() {
  lab::Webgpu webgpu("My Instance");
  lab::Shader shader("My Shader", "shaders/vbuf.wgsl");

  lab::Window window("A Window with tutorial code", 640, 400);
  lab::Surface surface(window, webgpu);

  WGPURenderPipeline pip = tutorial::InitializePipeline(webgpu, shader);
  auto res = tutorial::InitializeBuffers(webgpu);

  window.set_resize_callback(
      [&surface](int width, int height) { surface.reconfigure(width, height); });

  while (lab::tick()) {
    tutorial::MainLoop(webgpu, surface.wgpu_surface, pip, res.vertexBuffer, res.vertexCount);
    lab::sleep(50ms);
  }
}
