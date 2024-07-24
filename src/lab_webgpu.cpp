#include "lab_webgpu.h"

#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu/webgpu.hpp>

#include <glfw3webgpu.h>

namespace lab {

bool create_webgpu_instance(const Window& wnd) {
  wgpu::Instance instance = wgpu::createInstance({});
  if (!instance) {
    std::cerr << "Error: Could not initialize WebGPU!" << std::endl;
    return false;
  }
  std::cout << "WGPU instance: " << instance << std::endl;

  wgpu::Surface surface = glfwGetWGPUSurface(instance, reinterpret_cast<GLFWwindow*>(wnd.get_handle()));

  wgpu::RequestAdapterOptions adapterOpts = {{
      .compatibleSurface = surface,
      .powerPreference = wgpu::PowerPreference::HighPerformance,
  }};
  wgpu::Adapter adapter = instance.requestAdapter(adapterOpts);
  std::cout << "Got adapter: " << adapter << std::endl;

  instance.release();

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

  auto dims = wnd.get_dimensions();

  wgpu::SurfaceConfiguration surfaceConfig = {{
      .device = device,
      .format = surfaceCapabilities.formats[0],
      .usage = wgpu::TextureUsage::RenderAttachment,
      .alphaMode = wgpu::CompositeAlphaMode::Auto,
      .width = static_cast<uint32_t>(dims.width),
      .height = static_cast<uint32_t>(dims.height),
      .presentMode = wgpu::PresentMode::Fifo,
  }};
  surface.configure(surfaceConfig);

  // Describe and create shader module and render pipeline ---------------------
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

  return true;
}

} // namespace lab
