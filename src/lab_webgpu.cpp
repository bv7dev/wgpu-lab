#include "lab_webgpu.h"

#include <glfw3webgpu.h>

namespace lab {

Webgpu::Webgpu() {}

void Webgpu::init() {
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
      .label = "My device",
      .defaultQueue = {.label = "My default queue"},
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

  // set device error callback
  auto onDeviceError = [](wgpu::ErrorType type, char const* message) {
    std::cerr << "Error: WGPU: " << type;
    if (message) std::cerr << " (" << message << ")";
    std::cerr << std::endl;
  };
  auto cb = device.setUncapturedErrorCallback(onDeviceError);

  surface.getCapabilities(adapter, &capabilities);
  adapter.release();
}

wgpu::Surface Webgpu::configure_surface(wgpu::Surface new_surface, uint32_t width, uint32_t height) {
  wgpu::Surface old_surface = surface;
  if (new_surface) surface = new_surface;
  wgpu::SurfaceConfiguration surfaceConfig = {{
      .device = device,
      .format = capabilities.formats[0],
      .usage = wgpu::TextureUsage::RenderAttachment,
      .alphaMode = wgpu::CompositeAlphaMode::Auto,
      .width = width,
      .height = height,
      .presentMode = wgpu::PresentMode::Fifo,
  }};
  surface.configure(surfaceConfig);
  return old_surface;
}

void Webgpu::create_pipeline() {
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

  wgpu::ShaderModuleWGSLDescriptor shaderCodeDesc = {{
      .chain = {.next = nullptr, .sType = wgpu::SType::ShaderModuleWGSLDescriptor},
      .code = _SHADER_SOURCE,
  }};

  wgpu::ShaderModuleDescriptor shaderDesc;
  shaderDesc.nextInChain = &shaderCodeDesc.chain;
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

bool Webgpu::render_frame() {
  if (!instance) return false;

  wgpu::SurfaceTexture surfaceTexture;
  surface.getCurrentTexture(&surfaceTexture);
  if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_Success) {
    std::cerr << "Error: WGPU: could not get current texture" << std::endl;
    return false;
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
    return false;
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

  // Finally encode and submit the render pass
  wgpu::CommandBufferDescriptor cmdBufferDescriptor = {{.label = "My command buffer"}};
  wgpu::CommandBuffer commands = encoder.finish(cmdBufferDescriptor);
  encoder.release();

  queue.submit(commands);
  commands.release();

  targetView.release();
  surface.present();
  device.tick();

  return true;
}

Webgpu::~Webgpu() {
  if (instance) {
    std::cout << "Info: WGPU: Release: " << instance << std::endl;

    pipeline.release();
    surface.unconfigure();
    queue.release();
    surface.release();
    device.release();
    instance.release();

    instance = nullptr;
  }
}

} // namespace lab
