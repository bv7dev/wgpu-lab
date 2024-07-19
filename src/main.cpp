#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu/webgpu.hpp>

#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>

#include "webgpu-utils.h"

#include <iostream>

constexpr int WND_WIDTH = 640;
constexpr int WND_HEIGHT = 400;

const char* _SHADER_SOURCE = R"(
@vertex
fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> @builtin(position) vec4f {
    var p = vec2f(0.0, 0.0);
    if (in_vertex_index == 0u) {
        p = vec2f(-0.5, -0.5);
    } else if (in_vertex_index == 1u) {
        p = vec2f(0.5, -0.5);
    } else {
        p = vec2f(0.0, 0.5);
    }
    return vec4f(p, 0.0, 1.0);
}

@fragment
fn fs_main() -> @location(0) vec4f {
    return vec4f(0.0, 0.4, 1.0, 1.0);
}
)";

int main(int argc, char** argv) {
  std::cout << "Hi!" << std::endl;
  for (int i = 0; i < argc; ++i)
    std::cout << argv[i] << ' ';
  std::cout << std::endl;

  // GLFW Init -----------------------------------------------------------------
  if (!glfwInit()) return 1;
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  GLFWwindow* window = glfwCreateWindow(WND_WIDTH, WND_HEIGHT, "Hi!", nullptr, nullptr);
  if (!window) {
    std::cerr << "Error: Could not initialize GLFW!" << std::endl;
    glfwTerminate();
    return 1;
  }

  // WGPU Init -----------------------------------------------------------------
  wgpu::Instance instance = wgpu::createInstance({});
  if (!instance) {
    std::cerr << "Error: Could not initialize WebGPU!" << std::endl;
    return 1;
  }
  std::cout << "WGPU instance: " << instance << std::endl;

  wgpu::Surface surface = glfwGetWGPUSurface(instance, window);

  wgpu::RequestAdapterOptions adapterOpts = {{
      .compatibleSurface = surface,
      .powerPreference = wgpu::PowerPreference::HighPerformance,
  }};
  wgpu::Adapter adapter = instance.requestAdapter(adapterOpts);
  std::cout << "Got adapter: " << adapter << std::endl;

  instance.release();

  inspectAdapter(adapter);

  wgpu::DeviceDescriptor deviceDesc = {{
      .label = "My device",
      .defaultQueue = {.label = "My default queue"},
      .deviceLostCallbackInfo =
          {
              .mode = wgpu::CallbackMode::AllowSpontaneous,
              .callback =
                  [](const WGPUDevice* device, WGPUDeviceLostReason reason, char const* message, void*) {
                    std::cout << "Device " << device << " lost: reason " << reason;
                    if (message) std::cout << " (" << message << ")";
                    std::cout << std::endl;
                  },
          },
  }};
  wgpu::Device device = adapter.requestDevice(deviceDesc);
  std::cout << "Got device: " << device << std::endl;

  inspectDevice(device);

  // set device error callback
  auto onDeviceError = [](wgpu::ErrorType type, char const* message) {
    std::cerr << "Error: Uncaptured device error: type " << type;
    if (message) std::cerr << " (" << message << ")";
    std::cerr << std::endl;
  };
  auto cb = device.setUncapturedErrorCallback(onDeviceError);

  wgpu::SurfaceCapabilities surfaceCapabilities;
  surface.getCapabilities(adapter, &surfaceCapabilities);

  adapter.release();

  wgpu::SurfaceConfiguration surfaceConfig = {{
      .device = device,
      .format = surfaceCapabilities.formats[0],
      .usage = wgpu::TextureUsage::RenderAttachment,
      .alphaMode = wgpu::CompositeAlphaMode::Auto,
      .width = WND_WIDTH,
      .height = WND_HEIGHT,
      .presentMode = wgpu::PresentMode::Fifo,
  }};
  surface.configure(surfaceConfig);

  // Describe shader module ----------------------------------------------------
  wgpu::ShaderModuleDescriptor shaderDesc;

  wgpu::ShaderModuleWGSLDescriptor shaderCodeDesc;
  // Set the chained struct's header
  shaderCodeDesc.chain.next = nullptr;
  shaderCodeDesc.chain.sType = wgpu::SType::ShaderModuleWGSLDescriptor;
  shaderCodeDesc.code = _SHADER_SOURCE;
  // Connect the chain
  shaderDesc.nextInChain = &shaderCodeDesc.chain;

  wgpu::ShaderModule shaderModule = device.createShaderModule(shaderDesc);

  // Describe render pipeline --------------------------------------------------
  wgpu::RenderPipelineDescriptor pipelineDesc;

  // describe memory layout of buffers
  // first example not using this
  pipelineDesc.layout = nullptr;

  // We do not use any vertex buffer for this first simplistic example
  pipelineDesc.vertex.bufferCount = 0;
  pipelineDesc.vertex.buffers = nullptr;

  // Here we tell that the programmable vertex shader stage is described
  // by the function called 'vs_main' in that module.
  pipelineDesc.vertex.module = shaderModule;
  pipelineDesc.vertex.entryPoint = "vs_main";
  pipelineDesc.vertex.constantCount = 0;
  pipelineDesc.vertex.constants = nullptr;

  // Each sequence of 3 vertices is considered as a triangle
  pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;

  // We'll see later how to specify the order in which vertices should be
  // connected. When not specified, vertices are considered sequentially.
  pipelineDesc.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;

  // The face orientation is defined by assuming that when looking
  // from the front of the face, its corner vertices are enumerated
  // in the counter-clockwise (CCW) order.
  pipelineDesc.primitive.frontFace = wgpu::FrontFace::CCW;

  // But the face orientation does not matter much because we do not
  // cull (i.e. "hide") the faces pointing away from us (which is often
  // used for optimization).
  pipelineDesc.primitive.cullMode = wgpu::CullMode::None;

  // We tell that the programmable fragment shader stage is described
  // by the function called 'fs_main' in the shader module.
  wgpu::FragmentState fragmentState;
  fragmentState.module = shaderModule;
  fragmentState.entryPoint = "fs_main";
  fragmentState.constantCount = 0;
  fragmentState.constants = nullptr;

  // Fragment blending
  wgpu::BlendState blendState;

  // Configure color blending equation
  blendState.color.srcFactor = wgpu::BlendFactor::SrcAlpha;
  blendState.color.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
  blendState.color.operation = wgpu::BlendOperation::Add;

  // Configure alpha blending equation
  blendState.alpha.srcFactor = wgpu::BlendFactor::Zero;
  blendState.alpha.dstFactor = wgpu::BlendFactor::One;
  blendState.alpha.operation = wgpu::BlendOperation::Add;

  wgpu::ColorTargetState colorTarget;
  colorTarget.format = surfaceCapabilities.formats[0];
  colorTarget.blend = &blendState;
  colorTarget.writeMask = wgpu::ColorWriteMask::All; // We could write to only some of the color channels.

  // We have only one target because our render pass has only one output color
  // attachment.
  fragmentState.targetCount = 1;
  fragmentState.targets = &colorTarget;

  pipelineDesc.fragment = &fragmentState;

  // Samples per pixel
  pipelineDesc.multisample.count = 1;
  // Default value for the mask, meaning "all bits on"
  pipelineDesc.multisample.mask = ~0u;
  // Default value as well (irrelevant for count = 1 anyways)
  pipelineDesc.multisample.alphaToCoverageEnabled = false;

  // We do not use stencil/depth testing for now
  pipelineDesc.depthStencil = nullptr;

  wgpu::RenderPipeline pipeline = device.createRenderPipeline(pipelineDesc);

  // shader module can be released after pipeline creation
  shaderModule.release();

  wgpu::Queue queue = device.getQueue();

  // Main Loop -----------------------------------------------------------------
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    wgpu::TextureView targetView = getNextSurfaceTextureView(surface);
    if (!targetView) {
      std::cerr << "Error: Could not get next surface texture view" << std::endl;
      return 1;
    }

    // Create a command encoder for the draw call
    wgpu::CommandEncoderDescriptor encoderDesc = {{.label = "My command encoder"}};
    wgpu::CommandEncoder encoder = device.createCommandEncoder(encoderDesc);

    // The attachment part of the render pass descriptor describes the target
    // texture of the pass
    wgpu::RenderPassColorAttachment renderPassColorAttachment = {{
        .view = targetView,
        .depthSlice = WGPU_DEPTH_SLICE_UNDEFINED,
        .loadOp = wgpu::LoadOp::Clear,
        .storeOp = wgpu::StoreOp::Store,
        .clearValue = WGPUColor{0.9, 0.1, 0.2, 1.0},
    }};

    // Create the render pass that clears the screen with our color
    wgpu::RenderPassDescriptor renderPassDesc = {{
        .label = "My render pass",
        .colorAttachmentCount = 1,
        .colorAttachments = &renderPassColorAttachment,
    }};

    // Create the render pass and end it immediately (we only clear the screen
    // but do not draw anything)
    wgpu::RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);

    // Select which render pipeline to use
    renderPass.setPipeline(pipeline);
    // Draw 1 instance of a 3-vertices shape
    renderPass.draw(3, 1, 0, 0);

    renderPass.end();
    renderPass.release();

    // Finally encode and submit the render pass
    wgpu::CommandBufferDescriptor cmdBufferDescriptor = {{.label = "My command buffer"}};
    wgpu::CommandBuffer commands = encoder.finish(cmdBufferDescriptor);
    encoder.release();

    queue.submit(commands);
    commands.release();

    targetView.release();
    surface.present();
    device.tick();
  }

  // Cleanup -------------------------------------------------------------------
  surface.unconfigure();
  queue.release();
  surface.release();
  device.release();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
