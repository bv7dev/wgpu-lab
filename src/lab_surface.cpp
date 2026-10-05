#include "lab_detail.h"

#include "platform/lab_platform.h"

#include <GLFW/glfw3.h>

#include <algorithm>
#include <cstdlib>
#include <span>

namespace lab {

namespace detail {

SurfaceState::~SurfaceState() {
  view = nullptr;
  texture = nullptr;
  if (handle && configured_size != Size{}) {
    handle.Unconfigure();
  }
}

Size SurfaceState::framebuffer_size() const {
  Size size;
  glfwGetFramebufferSize(window->handle, &size.width, &size.height);
  return size;
}

void SurfaceState::configure(Size size) {
  wgpu::SurfaceConfiguration config{
      .device = gpu->device,
      .format = format,
      // a surface that is captured has to be readable, too
      .usage = capture_path.empty() ? wgpu::TextureUsage::RenderAttachment
                                    : wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc,
      .width = static_cast<uint32_t>(size.width),
      .height = static_cast<uint32_t>(size.height),
      .alphaMode = alpha_mode,
      .presentMode = present_mode,
  };
  handle.Configure(&config);
  configured_size = size;

  if (depth_format != wgpu::TextureFormat::Undefined) {
    wgpu::TextureDescriptor depth_desc{
        .label = "lab surface depth buffer",
        .usage = wgpu::TextureUsage::RenderAttachment,
        .size = {static_cast<uint32_t>(size.width), static_cast<uint32_t>(size.height), 1},
        .format = depth_format,
    };
    depth_texture = gpu->device.CreateTexture(&depth_desc);
  }
}

wgpu::TextureView SurfaceState::current_view() {
  if (view) {
    return view;
  }

  // a minimized window has no area to render onto
  const Size size = framebuffer_size();
  if (size.width <= 0 || size.height <= 0) {
    return nullptr;
  }
  if (size != configured_size) {
    configure(size);
  }

  wgpu::SurfaceTexture surface_texture;
  handle.GetCurrentTexture(&surface_texture);
  switch (surface_texture.status) {
  case wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal:
  case wgpu::SurfaceGetCurrentTextureStatus::SuccessSuboptimal:
    break;
  case wgpu::SurfaceGetCurrentTextureStatus::Timeout:
    return nullptr; // try again next frame
  case wgpu::SurfaceGetCurrentTextureStatus::Outdated:
  case wgpu::SurfaceGetCurrentTextureStatus::Lost:
    // the window changed under the surface: set it up again and skip this frame
    surface_texture.texture = nullptr;
    configured_size = {};
    return nullptr;
  default:
    fail(label, "could not get a texture to render into");
  }

  texture = std::move(surface_texture.texture);
  view = texture.CreateView();
  return view;
}

void SurfaceState::present() {
  if (!view) {
    return;
  }
  if (!capture_path.empty() && ++frames_presented == capture_frame) {
    save_texture_png(*gpu, texture, capture_path, label);
  }
  view = nullptr;
  texture = nullptr;
  handle.Present();
  Runtime::get().presented = true;
}

} // namespace detail

Surface::Surface(Gpu& gpu, Window& window, SurfaceOptions options)
    : shared_state{std::make_shared<detail::SurfaceState>()} {
  detail::SurfaceState& state = *shared_state;
  state.gpu = gpu.state();
  state.window = window.state();
  state.label = std::format("surface of window \"{}\"", glfwGetWindowTitle(window.handle()));
  state.present_mode = options.present_mode;
  state.depth_format = options.depth;

  state.handle = platform::create_surface(gpu.instance(), window.handle());
  if (!state.handle) {
    detail::fail(state.label, "could not be created");
  }

  wgpu::SurfaceCapabilities capabilities;
  state.handle.GetCapabilities(gpu.adapter(), &capabilities);
  const std::span formats{capabilities.formats, capabilities.formatCount};
  if (formats.empty()) {
    detail::fail(state.label, std::format("{} cannot render to this window", gpu.state()->label));
  }
  if (options.format == wgpu::TextureFormat::Undefined) {
    // An 8-bit format if there is one, like `navigator.gpu.getPreferredCanvasFormat()` on the web.
    // The format a window system lists first can be a wide one (RGBA16Float on HDR
    // capable desktops), which costs more and shows the same colors differently.
    const auto eight_bit = std::ranges::find_if(formats, [](wgpu::TextureFormat format) {
      return format == wgpu::TextureFormat::BGRA8Unorm || format == wgpu::TextureFormat::RGBA8Unorm;
    });
    state.format = eight_bit != formats.end() ? *eight_bit : formats.front();
  } else if (std::ranges::find(formats, options.format) != formats.end()) {
    state.format = options.format;
  } else {
    detail::fail(state.label, "the requested texture format is not supported by this window");
  }

  // frame capture, see SurfaceState
  if (const char* directory = std::getenv("LAB_CAPTURE_DIR")) {
    const char* frame = std::getenv("LAB_CAPTURE_FRAME");
    state.capture_frame = frame ? std::atol(frame) : 30;

    // named after the program, numbered if it has more than one surface
    const int number = ++detail::Runtime::get().surfaces_created;
    std::string name = platform::executable_path().stem().string();
    if (number > 1) {
      name += std::format("_{}", number);
    }
    std::error_code ignored;
    std::filesystem::create_directories(directory, ignored);
    state.capture_path = std::filesystem::path(directory) / (name + ".png");
  }

  const Size size = state.framebuffer_size();
  if (size.width > 0 && size.height > 0) {
    state.configure(size);
  }
}

wgpu::TextureFormat Surface::format() const { return shared_state->format; }
wgpu::TextureFormat Surface::depth_format() const { return shared_state->depth_format; }

Size Surface::size() const { return shared_state->framebuffer_size(); }

float Surface::aspect() const {
  const Size size = shared_state->framebuffer_size();
  return size.height > 0 ? static_cast<float>(size.width) / static_cast<float>(size.height) : 1.0f;
}

wgpu::TextureView Surface::current_view() { return shared_state->current_view(); }
wgpu::TextureView Surface::depth_view() {
  return shared_state->depth_texture ? shared_state->depth_texture.CreateView() : nullptr;
}
void Surface::present() { shared_state->present(); }

const wgpu::Surface& Surface::handle() const { return shared_state->handle; }

} // namespace lab
