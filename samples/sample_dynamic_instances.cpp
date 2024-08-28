#include <lab>

struct MyInstance {
  float x, y;
};

struct MyVertex {
  float x, y;
  float u, v;
};

struct MyPixel {
  uint8_t r, g, b, a;
};

struct MyUniform {
  float ratio[2];
  float time;
  float scale;
};

int main() {
  // ---------------------------------------------------------------------------
  // Window and controls -------------------------------------------------------
  lab::Window window("Dynamic Instances Demo - Use arrow keys to move", 640, 400);

  bool arrow_keys[4] = {};
  float input_axis_x = 0.f, input_axis_y = 0.f;

  window.set_key_callback([&arrow_keys, &input_axis_x, &input_axis_y](lab::KeyEvent event) {
    if (event.action != lab::InputType::repeat) {
      if (event.key >= lab::KeyCode::right && event.key <= lab::KeyCode::up) {
        const bool keydown = event.action == lab::InputType::press;
        arrow_keys[static_cast<uint32_t>(event.key) - static_cast<uint32_t>(lab::KeyCode::right)] = keydown;
        input_axis_x = static_cast<float>(arrow_keys[0] - arrow_keys[1]);
        input_axis_y = static_cast<float>(arrow_keys[3] - arrow_keys[2]);
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
  std::vector<MyPixel> pixel_data;
  pixel_data.reserve(texture.width() * texture.height());

  auto u8 = [](auto s) { return static_cast<uint8_t>(s); };
  auto f32 = [](auto s) { return static_cast<float>(s); };
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

  // 3 vertices of equilateral triangle     x        y       u    v
  std::vector<MyVertex> vertex_data = {
      {0.f, 1.f, 0.f, 0.f}, {-sqrtf(3.f) / 2.f, -3.f / 6.f, 0.f, 1.f}, {+sqrtf(3.f) / 2.f, -3.f / 6.f, 1.f, 1.f}};

  lab::Buffer<MyVertex> vertex_buffer("My vertex buffer", vertex_data, webgpu);

  // 5 triangle instance positions           x    y      x    y
  std::vector<MyInstance> instance_data = {{0.f, 0.f}, {.5f, .5f}, {-.2f, .4f}, {-.7f, -.2f}, {.5f, -.6f}};
  lab::Buffer<MyInstance> instance_buffer("My instance buffer", instance_data,
                                          wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst, webgpu);

  pipeline.add_vertex_buffer(vertex_buffer);
  pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32x2, 0); // pos
  pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32x2, 1); // uv

  pipeline.add_vertex_buffer(instance_buffer, wgpu::VertexStepMode::Instance);
  pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32x2, 2); // inst_pos

  MyUniform uniforms{.ratio = {window.ratio(), 1.0f}, .time = 0.0f, .scale = 0.2f};
  lab::Buffer<MyUniform> uniform_buffer("My uniform buffer", {uniforms},
                                        wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst, webgpu);

  pipeline.add_uniform_buffer(uniform_buffer, 0, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment);
  pipeline.finalize();

  float delta_t = 0.f;
  float vel_x = 0.f, vel_y = 0.f;
  float force = .04f, friction = .01f;

  // ---------------------------------------------------------------------------
  // Main loop -----------------------------------------------------------------
  float t0 = lab::elapsed_seconds();
  while (lab::tick(webgpu)) {
    pipeline.render_frame(surface, vertex_data.size(), instance_data.size());

    vel_x += input_axis_x * force, vel_y += input_axis_y * force;
    instance_data[0].x += vel_x * delta_t;
    instance_data[0].y += vel_y * delta_t;
    instance_buffer.write(instance_data[0]);

    vel_x *= 1.f - friction;
    vel_y *= 1.f - friction;

    uniform_buffer.write(uniforms);
    uniforms.ratio[0] = window.ratio();

    float t1 = lab::elapsed_seconds();
    uniforms.time = t1;
    delta_t = t1 - t0;
    t0 = t1;
  }
}
