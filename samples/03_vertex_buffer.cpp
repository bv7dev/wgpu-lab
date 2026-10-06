// 03: vertices come from a buffer (this is the example from the README)

#include <lab>

#include <vector>

using enum wgpu::VertexFormat;

struct MyVertex {
  float pos[2];
  float color[3];
};

int main() {
  lab::Gpu gpu;
  lab::Window window("Hello Triangle", 640, 400);
  lab::Surface surface(gpu, window);

  // colored triangle data
  std::vector<MyVertex> vertex_data = {
      //         X      Y                R     G     B
      {.pos = {-0.5f, -0.5f}, .color = {0.8f, 0.2f, 0.2f}},
      {.pos = {+0.5f, -0.5f}, .color = {0.8f, 0.8f, 0.2f}},
      {.pos = {+0.0f, +0.5f}, .color = {0.2f, 0.8f, 0.4f}},
  };

  // vertex buffer (sends a copy of the data to GPU memory)
  lab::Buffer vertices(gpu, vertex_data);

  lab::Shader shader(gpu, "shaders/03_vertex_buffer.wgsl");

  // the pipeline needs to know how a vertex is laid out in memory:
  // two floats for @location(0) position, three floats for @location(1) color
  lab::Pipeline pipeline(gpu, shader,
                         {.vertex_buffers = {lab::vertex<MyVertex>({Float32x2, Float32x3})}, .target = surface});

  // main application loop
  while (lab::tick()) {
    pipeline.render_frame(surface, {.vertex_buffers = {vertices}}); // draws all 3 vertices of the buffer
  }
}
