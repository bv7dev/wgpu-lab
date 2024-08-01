#include <lab>

// todo: it seems like my webgpu error callback is not working, so i don't get any error messages

int main() {
  lab::Webgpu webgpu("My Instance");
  lab::Shader shader("My Shader", "shaders/vbuf.wgsl");

  lab::Window window("A Window with tutorial code", 640, 400);

  std::vector<float> vertex_data = {-0.5,   -0.5, +0.5,   -0.5, +0.0,   +0.5,
                                    -0.55f, -0.5, -0.05f, +0.5, -0.55f, +0.5};
  uint32_t vertex_count = static_cast<uint32_t>(vertex_data.size() / 2);

  lab::ReadableBuffer<float> vertex_buffer("My vertex buffer", vertex_data, webgpu);

  lab::Surface surface(window, webgpu);
  lab::Pipeline pipeline(shader, webgpu);

  // todo: add methods on config to simplify this?
  pipeline.config.vertexAttribute_1.shaderLocation = 0;
  pipeline.config.vertexAttribute_1.format = wgpu::VertexFormat::Float32x2;
  pipeline.config.vertexBufferLayout.arrayStride = 2 * sizeof(float);
  pipeline.config.vertexBufferLayout.stepMode = wgpu::VertexStepMode::Vertex;

  pipeline.init(vertex_buffer.wgpu_buffer);

  // Idea: pipeline.configure(vbuf) to launch second init stages.
  //       It will call init() first. Note the similarity to surface resize api, maybe also add
  //       reconfigure() if it makes sense. This stage will be used to setup custom vertex attribs.
  //
  // Alternative: pipeline.use(vbuf) with auto configure if necessary?

  window.set_resize_callback(
      [&surface](int width, int height) { surface.reconfigure(width, height); });

  while (lab::tick()) {
    pipeline.render_frame(surface, {vertex_count, 1});
    lab::sleep(50ms);
  }
}
