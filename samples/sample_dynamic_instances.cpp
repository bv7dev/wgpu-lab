#include <lab>

int main() {
  struct MyInstancePos {
    float x, y;
  };

  struct MyVertexFormat {
    float x, y;
    float u, v;
  };

  struct MyPixelColor {
    uint8_t r, g, b, a;
  };

  struct MyUniformData {
    float ratio[2];
    float time;
    float scale;
  };

  struct MyInputMap {
    bool arrows[4];
    float axis_x, axis_y;
    void update(lab::KeyCode key, bool state) {
      arrows[static_cast<int>(key) - static_cast<int>(lab::KeyCode::right)] = state;
      axis_x = static_cast<float>(arrows[0] - arrows[1]);
      axis_y = static_cast<float>(arrows[3] - arrows[2]);
    }
  } input{};

  auto f32 = [](auto s) { return static_cast<float>(s); };
  auto u32 = [](auto s) { return static_cast<uint32_t>(s); };
  auto u8 = [](auto s) { return static_cast<uint8_t>(s); };

  // ---------------------------------------------------------------------------
  // Window and controls -------------------------------------------------------
  lab::Window window("Dynamic Instances Demo - Use arrow keys to move", 640, 400);

  window.set_key_callback([&window, &input](lab::KeyEvent event) {
    if (event.action != lab::KeyAction::repeat) {
      if (event.key >= lab::KeyCode::right && event.key <= lab::KeyCode::up) {
        input.update(event.key, event.action == lab::KeyAction::press);
      } else if (event.key == lab::KeyCode::escape) {
        window.close();
      }
    }
  });

  // ---------------------------------------------------------------------------
  // Webgpu Setup --------------------------------------------------------------
  lab::Webgpu webgpu("My Instance");
  lab::Surface surface(window, webgpu);

  lab::Shader shader("My Shader", "shaders/sample_dynamic_instances.wgsl");
  lab::Pipeline pipeline(shader, webgpu);

  lab::Texture texture(webgpu, wgpu::TextureFormat::RGBA8Unorm, 256, 256);

  // create procedural texture and upload to gpu
  std::vector<MyPixelColor> pixel_data;
  pixel_data.reserve(texture.width() * texture.height());

  auto transform = [&u8, &f32](int v, float s = 1.f) {
    return u8(s * (20.f + u8(sin(f32(v) / 255.f * 20.f) + 1.f) * 40.f));
  };

  for (int y = 0; y < texture.height(); ++y) {
    for (int x = 0; x < texture.width(); ++x) {
      pixel_data.emplace_back(transform(x + y, .6f), transform(x, .3f), transform(y, .2f), u8(255));
    }
  }
  texture.to_device(pixel_data);
  pipeline.add_texture(texture, 1, wgpu::ShaderStage::Fragment);

  // vertices of equilateral triangle
  //    x    y    u    v
  std::vector<MyVertexFormat> vertex_data = {
      {0.f, 1.f, 0.f, 0.f}, {-sqrtf(3.f) / 2.f, -3.f / 6.f, 0.f, 1.f}, {+sqrtf(3.f) / 2.f, -3.f / 6.f, 1.f, 1.f}};
  lab::Buffer<MyVertexFormat> vertex_buffer("My vertex buffer", vertex_data, webgpu);

  // triangle instance positions               x    y      x    y
  std::vector<MyInstancePos> instance_data = {{0.f, 0.f}, {.5f, .5f}, {-.2f, .4f}, {-.7f, -.2f}, {.5f, -.6f}};
  lab::Buffer<MyInstancePos> instance_buffer("My instance buffer", instance_data,
                                             wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst, webgpu);

  pipeline.add_vertex_buffer(vertex_buffer);
  pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32x2, 0); // pos
  pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32x2, 1); // uv

  pipeline.add_vertex_buffer(instance_buffer, wgpu::VertexStepMode::Instance);
  pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32x2, 2); // inst_pos

  MyUniformData uniforms{.ratio = {window.ratio(), 1.f}, .scale = .2f};
  lab::Buffer<MyUniformData> uniform_buffer("My uniform buffer", {uniforms},
                                            wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst, webgpu);

  pipeline.add_uniform_buffer(uniform_buffer, 0, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment);
  pipeline.finalize();

  // ---------------------------------------------------------------------------
  // Main loop -----------------------------------------------------------------
  const float force = .04f, friction = .01f;
  float velocity_x = 0.f, velocity_y = 0.f, delta_t = 0.f;

  float t0 = lab::elapsed_seconds();
  uniforms.time = t0;

  while (lab::tick()) {
    pipeline.render_frame(surface, vertex_data.size(), instance_data.size());

    velocity_x *= 1.f - friction;
    velocity_y *= 1.f - friction;
    velocity_x += input.axis_x * force;
    velocity_y += input.axis_y * force;

    instance_data[0].x += velocity_x * delta_t;
    instance_data[0].y += velocity_y * delta_t;
    instance_buffer.write(instance_data[0]);

    uniforms.ratio[0] = window.ratio();
    uniform_buffer.write(uniforms);

    uniforms.time = lab::elapsed_seconds();
    delta_t = uniforms.time - t0;
    t0 = uniforms.time;
  }
}
