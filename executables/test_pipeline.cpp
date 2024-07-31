#include <lab>

int main() {
  using namespace lab;

  Webgpu webgpu("My Instance");
  Shader shader("My Shader", "shaders/test1.wgsl");
  Window window("pipeline tests", 640, 400);

  Surface surface(window, webgpu);
  window.set_resize_callback(
      [&surface](int width, int height) { surface.reconfigure(width, height); });

  // Configure Render Pipeline -------------------------------------------------
  Pipeline pipeline(shader, webgpu);

  auto& cfg_prim = pipeline.config.primitiveState;
  cfg_prim.topology = wgpu::PrimitiveTopology::LineStrip;
  cfg_prim.stripIndexFormat = wgpu::IndexFormat::Uint32;

  // initialize after configuring
  pipeline.init();

  // render config only affects render_frame
  auto& cfg_color = pipeline.render_config.renderPassColorAttachment;
  cfg_color.clearValue = wgpu::Color(0.25, 0.2, 0.2, 1.0);

  while (tick()) {
    pipeline.render_frame(surface, {3, 1});
    lab::sleep(50ms);
  }
}
