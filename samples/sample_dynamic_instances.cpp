#include <glm/glm.hpp>
#include <lab>

int main() {
  struct MyInstancePos {
    glm::vec2 pos;
  };

  struct MyVertexFormat {
    glm::vec2 pos;
    glm::vec2 uv;
  };

  struct MyPixelFormat {
    uint8_t r, g, b, a;
  };

  struct MyUniformData {
    glm::vec2 ratio;
    float time, scale;
  };

  struct MyInputState {
    bool arrows[4];
    glm::vec2 axis;
    uint32_t current_instance;
    void update_axis(lab::KeyCode key, bool state) {
      arrows[static_cast<int>(key) - 262u] = state;
      axis.x = static_cast<float>(arrows[0] - arrows[1]);
      axis.y = static_cast<float>(arrows[3] - arrows[2]);
    }
  };

  // ---------------------------------------------------------------------------
  // Window and controls -------------------------------------------------------
  MyInputState input{};
  lab::Window window("Press arrow keys to move and space to cycle through", 640, 400);
  window.set_key_callback([&window, &input](lab::KeyEvent event) {
    if (event.action != lab::KeyAction::repeat) {
      if (event.key >= lab::KeyCode::right && event.key <= lab::KeyCode::up) {
        input.update_axis(event.key, event.action == lab::KeyAction::press);
      } else if (event.key == lab::KeyCode::space && event.action == lab::KeyAction::release) {
        ++input.current_instance;
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
  std::vector<MyVertexFormat> vertex_data = {{.pos{0.f, 1.f}, .uv{0.f, 0.f}},
                                             {.pos{-sqrtf(3.f) / 2.f, -3.f / 6.f}, .uv{0.f, 1.f}},
                                             {.pos{+sqrtf(3.f) / 2.f, -3.f / 6.f}, .uv{1.f, 1.f}}};
  lab::Buffer<MyVertexFormat> vertex_buffer("My vertex buffer", vertex_data, webgpu);

  // positions of triangle instances
  std::vector<MyInstancePos> instance_data = {{{0.f, 0.f}}, {{.5f, .5f}}, {{-.2f, .4f}}, {{-.7f, -.2f}}, {{.5f, -.6f}}};
  lab::Buffer<MyInstancePos> instance_buffer("My instance buffer", instance_data,
                                             wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst, webgpu);

  pipeline.add_vertex_buffer(vertex_buffer);
  pipeline.add_vertex_attrib(wgpu::VertexFormat::Float32x2, 0); // position
  pipeline.add_vertex_attrib(wgpu::VertexFormat::Float32x2, 1); // uv

  pipeline.add_vertex_buffer(instance_buffer, wgpu::VertexStepMode::Instance);
  pipeline.add_vertex_attrib(wgpu::VertexFormat::Float32x2, 2); // instance_position

  MyUniformData uniforms{.ratio = {window.ratio(), 1.f}, .scale = .2f};
  lab::Buffer<MyUniformData> uniform_buffer("My uniform buffer", {uniforms},
                                            wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst, webgpu);

  pipeline.add_uniform_buffer(uniform_buffer, 0, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment);
  pipeline.finalize();

  // ---------------------------------------------------------------------------
  // Main loop -----------------------------------------------------------------
  const float force = 4.f, friction = 1.f;
  glm::vec2 velocity = {0.f, 0.f};
  float deltatime = 0.f;

  float t0 = lab::elapsed_seconds();
  uniforms.time = t0;

  while (lab::tick()) {
    pipeline.render_frame(surface, int(vertex_data.size()), int(instance_data.size()));

    const int i = input.current_instance % instance_data.size();
    velocity = velocity * (1.f - friction * deltatime) + input.axis * force * deltatime;
    instance_data[i].pos += velocity * deltatime;
    instance_buffer.write(instance_data[i], i);

    uniforms.ratio = {window.ratio(), 1.f};
    uniform_buffer.write(uniforms);

    uniforms.time = lab::elapsed_seconds();
    deltatime = uniforms.time - t0;
    t0 = uniforms.time;
  }
}
