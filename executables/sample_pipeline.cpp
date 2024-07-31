#include <lab>

int main() {
  using namespace lab;

  Webgpu webgpu("My Instance");
  Shader shader("My Shader", "shaders/test1.wgsl");
  Window window("pipeline tests", 640, 400);

  Surface surface(window, webgpu);
  window.set_resize_callback(
      [&surface](int width, int height) { surface.reconfigure(width, height); });

  window.set_key_callback([&window](const KeyEvent& event) {
    if (event.key == KeyCode::escape) window.close();
  });

  // Configure Render Pipeline -------------------------------------------------
  Pipeline pipeline(shader, webgpu);

  // draw line strip
  auto& primitive_cfg = pipeline.config.primitiveState;
  primitive_cfg.topology = wgpu::PrimitiveTopology::LineStrip;
  primitive_cfg.stripIndexFormat = wgpu::IndexFormat::Uint32;

  // initialize after configuring
  pipeline.init();

  // render config only affects render_frame
  auto& color_cfg = pipeline.render_config.renderPassColorAttachment;
  color_cfg.clearValue = wgpu::Color(0.25, 0.2, 0.2, 1.0);

  while (tick()) {
    DrawCallParams draw(3, 1); // draw 3 vertices and 1 instance
    pipeline.render_frame(surface, draw);
    sleep(50ms);
  }
}
