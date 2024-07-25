// Tutorial: https://eliemichel.github.io/LearnWebGPU/

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

  // Describe and create shader module and render pipeline ---------------------
  wgpu::ShaderModuleWGSLDescriptor shaderCodeDesc = {{
      .chain = {.next = nullptr, .sType = wgpu::SType::ShaderModuleWGSLDescriptor},
      .code = _SHADER_SOURCE,
  }};

  wgpu::ShaderModuleDescriptor shaderDesc;
  shaderDesc.nextInChain = &shaderCodeDesc.chain; // connect the chain
  shaderDesc.label = "My shader module";

  wgpu::ShaderModule shaderModule = device.createShaderModule(shaderDesc);

  wgpu::BlendComponent blendColor = {{
      .operation = wgpu::BlendOperation::Add,
      .srcFactor = wgpu::BlendFactor::SrcAlpha,
      .dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha,
  }};

  wgpu::BlendComponent blendAlpha = {{
      .operation = wgpu::BlendOperation::Add,
      .srcFactor = wgpu::BlendFactor::Zero,
      .dstFactor = wgpu::BlendFactor::One,
  }};

  wgpu::BlendState blendState = {{
      .color = blendColor,
      .alpha = blendAlpha,
  }};

  wgpu::ColorTargetState colorTarget = {{
      .format = surfaceCapabilities.formats[0],
      .blend = &blendState,
      .writeMask = wgpu::ColorWriteMask::All,
  }};

  wgpu::FragmentState fragmentState = {{
      .module = shaderModule,
      .entryPoint = "fs_main",
      .constantCount = 0,
      .constants = nullptr,
      .targetCount = 1,
      .targets = &colorTarget,
  }};

  wgpu::RenderPipelineDescriptor pipelineDesc = {{
      .label = "My render pipeline",
      .vertex = {.module = shaderModule, .entryPoint = "vs_main"},
      .primitive = {.topology = wgpu::PrimitiveTopology::TriangleList,
                    .stripIndexFormat = wgpu::IndexFormat::Undefined,
                    .frontFace = wgpu::FrontFace::CCW,
                    .cullMode = wgpu::CullMode::None},
      .multisample = {.count = 1, .mask = ~0u},
      .fragment = &fragmentState,
  }};
  wgpu::RenderPipeline pipeline = device.createRenderPipeline(pipelineDesc);
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
  pipeline.release();
  surface.unconfigure();
  queue.release();
  surface.release();
  device.release();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
