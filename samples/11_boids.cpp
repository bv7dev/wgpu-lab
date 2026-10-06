// 11: a compute shader simulates a flock, the result is drawn in the same frame

#include <lab>

#include <glm/glm.hpp>

#include <algorithm>
#include <random>
#include <vector>

using enum wgpu::VertexFormat;

// has to match `struct Boid` in the shaders
struct Boid {
  glm::vec2 position;
  glm::vec2 velocity;
};

// has to match `struct Params` in the compute shader
struct Params {
  float delta_time = 0.0f;
  float cohesion_distance = 0.10f;
  float separation_distance = 0.025f;
  float alignment_distance = 0.06f;
  float cohesion_scale = 0.01f;
  float separation_scale = 0.05f;
  float alignment_scale = 0.03f;
};

int main() {
  lab::Gpu gpu;
  lab::Window window("Boids", 700, 700);
  lab::Surface surface(gpu, window);

  // a flock of boids at random places, flying in random directions
  const uint32_t boid_count = 2000;
  std::mt19937 random(42);
  std::uniform_real_distribution<float> anywhere(-1.0f, 1.0f);
  std::vector<Boid> boids(boid_count);
  for (Boid& boid : boids) {
    boid.position = {anywhere(random), anywhere(random)};
    boid.velocity = 0.1f * glm::vec2{anywhere(random), anywhere(random)};
  }

  // Two buffers: each step of the simulation reads one and writes the other, then they
  // swap roles. The compute shader sees them as storage buffers, the render pipeline
  // reads the very same buffers as per-instance vertex buffers.
  const wgpu::BufferUsage usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::Vertex;
  lab::Buffer<Boid> boid_buffers[2] = {lab::Buffer<Boid>(gpu, boids, usage), lab::Buffer<Boid>(gpu, boids, usage)};

  Params params;
  lab::Buffer<Params> params_buffer(gpu, {params}, wgpu::BufferUsage::Uniform);

  // the simulation
  lab::Shader simulate_shader(gpu, "shaders/11_boids_simulate.wgsl");
  lab::ComputePipeline simulate(gpu, simulate_shader);

  // one bind group per direction: [0] reads buffer 0 and writes buffer 1, [1] the other way round
  const wgpu::BindGroup step[2] = {
      simulate.bind_group(0, {{0, params_buffer}, {1, boid_buffers[0]}, {2, boid_buffers[1]}}),
      simulate.bind_group(0, {{0, params_buffer}, {1, boid_buffers[1]}, {2, boid_buffers[0]}}),
  };

  // drawing: a small triangle per boid
  std::vector<glm::vec2> boid_shape = {{-0.008f, -0.016f}, {0.008f, -0.016f}, {0.0f, 0.016f}};
  lab::Buffer shape(gpu, boid_shape);

  lab::Shader draw_shader(gpu, "shaders/11_boids_draw.wgsl");
  lab::Pipeline draw_pipeline(
      gpu, draw_shader,
      {.vertex_buffers = {lab::vertex<glm::vec2>({Float32x2}), lab::instance<Boid>({Float32x2, Float32x2})},
       .target = surface});

  uint32_t latest = 0; // which of the two buffers holds the current state

  while (lab::tick()) {
    if (window.key_pressed(lab::KeyCode::escape)) {
      window.close();
    }

    params.delta_time = std::min(lab::delta_seconds(), 0.05f); // a hiccup must not make the boids jump
    params_buffer.write(params);

    // a frame with two passes: first compute the next state, then draw it
    lab::Frame frame(gpu);
    {
      lab::ComputePass compute(frame);
      compute.dispatch(simulate, {step[latest]}, (boid_count + 63) / 64); // in workgroups of 64 boids
    }
    latest = 1 - latest;

    lab::RenderPass pass(frame, surface);
    pass.draw(draw_pipeline, {.vertex_buffers = {shape, boid_buffers[latest]}});
  }
}
