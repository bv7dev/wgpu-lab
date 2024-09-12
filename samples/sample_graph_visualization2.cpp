#include <lab>

#include <glm/glm.hpp>

#include <random>

struct Vertex {
  glm::vec2 pos;
  float side;
};

struct NodeInstance {
  glm::vec2 pos;
  float scale;
};

struct alignas(16) UniformParams {
  glm::vec2 ratio;
  float time;
};

int main() {
  lab::Webgpu webgpu("wgpu context");

  lab::Shader node_shader("node shader");
  node_shader.source = R"(
  struct Uniforms {
    ratio : vec2f, time : f32,
  };

  struct Vertex {
    @location(0) pos : vec2f,
    @location(1) side : f32,
  };

  struct Instance {
    @location(2) pos : vec2f,
    @location(3) scale : f32,
  };

  struct VsOutput {
    @builtin(position) position : vec4f,
    @location(0) pos : vec2f,
    @location(1) side: f32,
  };

  @group(0) @binding(0) var<uniform> uniforms : Uniforms;

  @vertex fn vs_main(vtx: Vertex, ins: Instance) -> VsOutput {
    var out : VsOutput;
    var offset = vec2f(0.0);
    if (vtx.side > 0.0) {
      offset.x += 0.4;
    }
    out.position = vec4f(((ins.pos + vtx.pos*ins.scale) + offset)*uniforms.ratio, 0.0, 1.0);
    out.pos = vtx.pos;
    out.side = vtx.side;
    return out;
  }

  @fragment fn fs_main(in : VsOutput) -> @location(0) vec4f {
    let intensity = pow(1.0 - length(in.pos), 2.0);

    // // colorize sides
    // if (intensity <= 0.02) {
    //   return vec4f(0.2, 0.0, 0.0, 0.2);
    // }
    // return vec4f(0.2 + intensity * 0.8)*(vec4f(1.0, 0.7, 0.7, 1.0) * max(in.side, 0.4)) +
    //        vec4f(0.2 + intensity * 0.8)*(vec4f(0.7, 0.7, 1.0, 1.0) * max(-in.side, 0.4));

    return vec4f(intensity);
  }
  )";

  lab::Pipeline node_pipeline(node_shader, webgpu);

  const float k = sqrt(3.0f) / 3.0f;
  std::vector<Vertex> mesh{
      {{-1.0f, k}, -1.0},   {{-1.0f, -k}, -1.0},   {{0.0f, -2 * k}, -1.0}, {{0.0f, 2 * k}, -1.0}, // left quad
      {{0.0f, 2 * k}, 1.0}, {{0.0f, -2 * k}, 1.0}, {{1.0f, -k}, 1.0},      {{1.0f, k}, 1.0},      // right quad
  };

  std::vector<uint16_t> mesh_indices{
      0, 1, 2, 2, 3, 0, // left quad
      3, 2, 5, 5, 4, 3, // central quad
      4, 5, 6, 6, 7, 4, // right quad
  };

  lab::Buffer mesh_vertex_buffer("mesh vertex buffer", mesh, webgpu);
  lab::Buffer mesh_index_buffer("mesh index buffer", mesh_indices, wgpu::BufferUsage::Index, webgpu);

  node_pipeline.add_vertex_buffer(mesh_vertex_buffer);
  node_pipeline.add_vertex_attrib(wgpu::VertexFormat::Float32x2, 0);
  node_pipeline.add_vertex_attrib(wgpu::VertexFormat::Float32, 1);

  node_pipeline.add_index_buffer(mesh_index_buffer, wgpu::IndexFormat::Uint16);

  std::vector<NodeInstance> node_instances;
  node_instances.push_back({.pos = {0.0f, 0.0f}, .scale = 0.08f});

  lab::Buffer<NodeInstance> node_instance_buffer("node instance buffer", node_instances, webgpu);

  node_pipeline.add_vertex_buffer(node_instance_buffer, wgpu::VertexStepMode::Instance);
  node_pipeline.add_vertex_attrib(wgpu::VertexFormat::Float32x2, 2);
  node_pipeline.add_vertex_attrib(wgpu::VertexFormat::Float32, 3);

  lab::Window window("graph visualizer", 900, 600);
  lab::Surface surface(window, webgpu);

  UniformParams uniform_params{.ratio{window.ratio(), 1.0}};
  lab::Buffer<UniformParams> uniform_buffer("node uniform buffer", {uniform_params},
                                            wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst, webgpu);

  node_pipeline.add_uniform_buffer(uniform_buffer, 0, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment);
  node_pipeline.finalize();

  window.set_key_callback([&window](auto event) {
    if (event.key == lab::KeyCode::escape) window.close();
  });

  window.set_resize_callback([&uniform_params, &uniform_buffer, &window, &surface](int w, int h) {
    std::cout << "window resized to: " << w << "x" << h << "\n";
    uniform_params.ratio.x = static_cast<float>(h) / w;
    uniform_buffer.write(uniform_params);
    surface.reconfigure(w, h);
  });

  while (lab::tick()) {
    wgpu::TextureView targetView = lab::get_current_render_texture_view(surface.wgpu_surface);

    wgpu::CommandEncoderDescriptor encoderDesc = {.label = "my command encoder"};
    wgpu::CommandEncoder encoder = node_pipeline.webgpu.device.CreateCommandEncoder(&encoderDesc);

    node_pipeline.render_config.renderPassColorAttachment.view = targetView;
    node_pipeline.render_config.renderPassDesc.colorAttachmentCount = 1;
    node_pipeline.render_config.renderPassDesc.colorAttachments =
        &node_pipeline.render_config.renderPassColorAttachment;

    wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&node_pipeline.render_config.renderPassDesc);
    renderPass.SetPipeline(node_pipeline.wgpu_pipeline);

    for (uint32_t i = 0; i < node_pipeline.vb_configs.size(); ++i) {
      renderPass.SetVertexBuffer(i, node_pipeline.vb_configs[i].buffer, node_pipeline.vb_configs[i].offset,
                                 node_pipeline.vb_configs[i].buffer.GetSize());
    }

    for (uint32_t i = 0; i < node_pipeline.ib_configs.size(); ++i) {
      const auto& ibc = node_pipeline.ib_configs[i];
      renderPass.SetIndexBuffer(ibc.buffer, ibc.format, ibc.offset, ibc.buffer.GetSize());
    }

    for (uint32_t i = 0; i < node_pipeline.bindGroups.size(); ++i) {
      renderPass.SetBindGroup(i, node_pipeline.bindGroups[i], 0, nullptr);
    }

    renderPass.DrawIndexed(uint32_t(mesh_indices.size()), uint32_t(node_instances.size()));

    renderPass.End();

    wgpu::CommandBufferDescriptor cmdBufferDescriptor = {.label = "my command buffer"};
    wgpu::CommandBuffer commands = encoder.Finish(&cmdBufferDescriptor);

    webgpu.queue.Submit(1, &commands);

    surface.wgpu_surface.Present();
  }
}
