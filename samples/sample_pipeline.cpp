#include <lab>

int main() {
  using namespace lab;

  Webgpu webgpu("My WebGPU Context");
  Shader shader("My Shader Module", "shaders/test1.wgsl");

  Window window("Triangle Wireframe Demo", 640, 400);

  Surface surface(window, webgpu);

  window.set_key_callback([&window](const KeyEvent& event) {
    if (event.key == KeyCode::escape) window.close();
  });

  // Configure Render Pipeline -------------------------------------------------
  Pipeline pipeline(shader, webgpu);

  // draw line strip
  auto& primitive_cfg = pipeline.config.primitiveState;
  primitive_cfg.topology = wgpu::PrimitiveTopology::LineStrip;
  primitive_cfg.stripIndexFormat = wgpu::IndexFormat::Uint32;

  // initialize after configuration
  pipeline.finalize();

  // render config only affects render_frame
  auto& color_cfg = pipeline.render_config.renderPassColorAttachment;
  color_cfg.clearValue = wgpu::Color(0.25, 0.2, 0.2, 1.0);

  while (tick()) {
    Pipeline::DrawCallParams draw_params(3, 1); // draw 3 vertices and 1 instance
    pipeline.render_frame(surface, draw_params);
  }
}
