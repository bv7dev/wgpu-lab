#include "lab_webgpu.h"

#include <glfw3webgpu.h>

namespace lab {

Webgpu::Webgpu(const char* lbl) : label{lbl} {
  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  GLFWwindow* window = glfwCreateWindow(1, 1, "", nullptr, nullptr);

  wgpu::Surface surface = glfwGetWGPUSurface(instance, window);

  instance = wgpu::createInstance({});
  if (!instance) {
    std::cerr << "Error: WGPU: Could not create Instance!" << std::endl;
    return;
  }
  std::cout << "Info: WGPU: Create: " << instance << std::endl;

  wgpu::RequestAdapterOptions adapterOpts = {{
      .compatibleSurface = surface,
      .powerPreference = wgpu::PowerPreference::HighPerformance,
  }};
  wgpu::Adapter adapter = instance.requestAdapter(adapterOpts);
  std::cout << "Info: WGPU: Request: " << adapter << std::endl;

  wgpu::DeviceDescriptor deviceDesc = {{
      .label = "default device",
      .defaultQueue = {.label = "default queue"},
      .deviceLostCallbackInfo =
          {
              .mode = wgpu::CallbackMode::AllowSpontaneous,
              .callback =
                  [](const WGPUDevice* device, WGPUDeviceLostReason reason, char const* message, void*) {
                    std::cout << "Warning: WGPU: Device " << device << " lost: reason " << reason;
                    if (message) std::cout << " (" << message << ")";
                    std::cout << std::endl;
                  },
          },
  }};
  device = adapter.requestDevice(deviceDesc);
  std::cout << "Info: WGPU: Request: " << device << std::endl;

  auto onDeviceError = [](wgpu::ErrorType type, char const* message) {
    std::cerr << "Error: WGPU: " << type;
    if (message) std::cerr << " (" << message << ")";
    std::cerr << std::endl;
  };
  auto cb = device.setUncapturedErrorCallback(onDeviceError);

  surface.getCapabilities(adapter, &capabilities);
  adapter.release();

  glfwDestroyWindow(window);
}

void Webgpu::create_pipeline(wgpu::ShaderModule shaderModule) {
  if (pipeline) pipeline.release();
  if (queue) queue.release();

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
      .format = capabilities.formats[0],
      .blend = &blendState,
      .writeMask = wgpu::ColorWriteMask::All,
  }};

  wgpu::FragmentState fragmentState = {{
      .module = shaderModule,
      .entryPoint = "fs_main",
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
  pipeline = device.createRenderPipeline(pipelineDesc);
  shaderModule.release();

  queue = device.getQueue();
}

void Webgpu::render_frame(wgpu::Surface surface) {
  if (!instance) return;

  wgpu::SurfaceTexture surfaceTexture;
  surface.getCurrentTexture(&surfaceTexture);
  if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_Success) {
    std::cerr << "Error: WGPU: could not get current texture" << std::endl;
    return;
  }
  WGPUTextureViewDescriptor viewDescriptor{
      .label = "My texture view",
      .format = wgpuTextureGetFormat(surfaceTexture.texture),
      .dimension = WGPUTextureViewDimension_2D,
      .baseMipLevel = 0,
      .mipLevelCount = 1,
      .baseArrayLayer = 0,
      .arrayLayerCount = 1,
      .aspect = WGPUTextureAspect_All,
  };
  wgpu::TextureView targetView = wgpuTextureCreateView(surfaceTexture.texture, &viewDescriptor);
  if (!targetView) {
    std::cerr << "Error: WGPU: Could not create texture view" << std::endl;
    return;
  }

  wgpu::CommandEncoderDescriptor encoderDesc = {{.label = "My command encoder"}};
  wgpu::CommandEncoder encoder = device.createCommandEncoder(encoderDesc);

  wgpu::RenderPassColorAttachment renderPassColorAttachment = {{
      .view = targetView,
      .depthSlice = WGPU_DEPTH_SLICE_UNDEFINED,
      .loadOp = wgpu::LoadOp::Clear,
      .storeOp = wgpu::StoreOp::Store,
      .clearValue = WGPUColor{0.9, 0.1, 0.2, 1.0},
  }};

  wgpu::RenderPassDescriptor renderPassDesc = {{
      .label = "My render pass",
      .colorAttachmentCount = 1,
      .colorAttachments = &renderPassColorAttachment,
  }};

  wgpu::RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);
  renderPass.setPipeline(pipeline);

  renderPass.draw(3, 1, 0, 0);

  renderPass.end();
  renderPass.release();

  wgpu::CommandBufferDescriptor cmdBufferDescriptor = {{.label = "My command buffer"}};
  wgpu::CommandBuffer commands = encoder.finish(cmdBufferDescriptor);
  encoder.release();

  queue.submit(commands);
  commands.release();

  targetView.release();
  surface.present();
  device.tick();
}

Webgpu::~Webgpu() {
  if (instance) {
    std::cout << "Info: WGPU: Release: " << instance << std::endl;

    pipeline.release();
    queue.release();
    device.release();
    instance.release();

    instance = nullptr;
  }
}

} // namespace lab
