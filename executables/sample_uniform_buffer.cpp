#include <lab>

struct MyVertex {
  float x, y;
};

struct MyUniforms {
  float ratio[2];
  float time;
  float scale;
};

int main() {
  lab::Webgpu webgpu("My Instance");
  lab::Window window("Vertex Buffer & Uniforms Demo", 640, 400);

  std::vector<MyVertex> vertex_data = {
      {0.f, 1.f}, {-sqrtf(3.f) / 2.f, -3.f / 6.f}, {sqrtf(3.f) / 2.f, -3.f / 6.f}};

  lab::Buffer<MyVertex> vertex_buffer("My vertex buffer", vertex_data, webgpu);

  MyUniforms uniforms{.ratio = {window.ratio(), 1.0f}, .time = 0.0f, .scale = 0.4f};
  lab::Buffer<MyUniforms> uniform_buffer("My uniform buffer", {uniforms},
                                         wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst,
                                         webgpu);

  lab::Shader shader("My Shader", "shaders/sample_uniform_buffer.wgsl");
  lab::Pipeline pipeline(shader, webgpu);

  pipeline.add_vertex_buffer(vertex_buffer);
  pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32x2, 0);

  pipeline.add_uniform_buffer(uniform_buffer, 0,
                              wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment);
  pipeline.finalize();

  lab::Surface surface(window, webgpu);

  std::thread time_resetter{[]() {
    std::cout << "Info: spawned a thread which resets time every 7 seconds" << std::endl;
    while (true) {
      lab::reset_time();
      while (lab::elapsed_seconds() < 7.0f) {
        lab::sleep(20ms);
      }
      std::cout << "time reset at: " << lab::elapsed_seconds() << std::endl;
    }
  }};

  while (lab::tick()) {
    // send uniforms to gpu
    uniform_buffer.write(uniforms);

    // render the scene
    pipeline.render_frame(surface, {3, 1});

    // update uniform values
    uniforms.ratio[0] = window.ratio();
    uniforms.time = lab::elapsed_seconds();

    lab::sleep(20ms);
  }

  time_resetter.join();
}
