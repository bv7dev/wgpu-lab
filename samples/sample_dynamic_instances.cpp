#include <lab>

int main() {
  struct MyInstancePos {
    float x, y;
  };

  struct MyVertexFormat {
    float x, y;
    float u, v;
  };

  struct MyPixelFormat {
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
      arrows[static_cast<int>(key) - 262u] = state;
      axis_x = static_cast<float>(arrows[0] - arrows[1]);
      axis_y = static_cast<float>(arrows[3] - arrows[2]);
    }
  } input{};

  // ---------------------------------------------------------------------------
  // Window and controls -------------------------------------------------------
  lab::Window window("Press arrow keys to move and space to cycle through", 640, 400);

  int current_instance_index = 0;
  window.set_key_callback([&window, &input, &current_instance_index](lab::KeyEvent event) {
    if (event.action != lab::KeyAction::repeat) {
      if (event.key >= lab::KeyCode::right && event.key <= lab::KeyCode::up) {
        input.update(event.key, event.action == lab::KeyAction::press);
      } else if (event.key == lab::KeyCode::space && event.action == lab::KeyAction::release) {
        ++current_instance_index;
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
  std::vector<MyPixelFormat> pixel_data;
  pixel_data.reserve(texture.width() * texture.height());

  auto transform = [](int v, float s = 1.f) {
    return uint8_t(s * (20.f + uint8_t(sin(float(v) / 255.f * 20.f) + 1.f) * 40.f));
  };

  for (int y = 0; y < texture.height(); ++y) {
    for (int x = 0; x < texture.width(); ++x) {
      pixel_data.emplace_back(transform(x + y, .6f), transform(x, .3f), transform(y, .2f), uint8_t(255u));
    }
  }
  texture.to_device(pixel_data);
  pipeline.add_texture(texture, 1, wgpu::ShaderStage::Fragment);

  // create vertex buffer with 3 vertices which form an equilateral triangle
  //    x    y    u    v                 x           y       u    v
  std::vector<MyVertexFormat> vertex_data = {
      {0.f, 1.f, 0.f, 0.f}, {-sqrtf(3.f) / 2.f, -3.f / 6.f, 0.f, 1.f}, {+sqrtf(3.f) / 2.f, -3.f / 6.f, 1.f, 1.f}};
  lab::Buffer<MyVertexFormat> vertex_buffer("My vertex buffer", vertex_data, webgpu);

  // triangle instance positions                x    y      x    y
  std::vector<MyInstancePos> instance_data = {{0.f, 0.f}, {.5f, .5f}, {-.2f, .4f}, {-.7f, -.2f}, {.5f, -.6f}};
  lab::Buffer<MyInstancePos> instance_buffer("My instance buffer", instance_data,
                                             wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst, webgpu);

  pipeline.add_vertex_buffer(vertex_buffer);
  pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32x2, 0); // position
  pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32x2, 1); // uv

  pipeline.add_vertex_buffer(instance_buffer, wgpu::VertexStepMode::Instance);
  pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32x2, 2); // instance_position

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

    int i = current_instance_index % instance_data.size();
    instance_data[i].x += velocity_x * delta_t;
    instance_data[i].y += velocity_y * delta_t;
    instance_buffer.write(instance_data[i], i);

    uniforms.ratio[0] = window.ratio();
    uniform_buffer.write(uniforms);

    uniforms.time = lab::elapsed_seconds();
    delta_t = uniforms.time - t0;
    t0 = uniforms.time;
  }
}
