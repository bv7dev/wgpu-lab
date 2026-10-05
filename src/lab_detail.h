#ifndef WGPU_LAB_DETAIL_H
#define WGPU_LAB_DETAIL_H

// Private to the library: the state behind the public objects.
//
// No lab object holds a reference to another lab object. They share these state
// structs through shared_ptr instead, so the order in which a program destroys
// its objects does not matter.

#include <lab_error.h>
#include <lab_gpu.h>
#include <lab_input.h>
#include <lab_log.h>
#include <lab_surface.h>
#include <lab_window.h>

#include <bitset>
#include <chrono>
#include <filesystem>
#include <format>
#include <functional>
#include <optional>
#include <string>
#include <vector>

struct GLFWwindow;

namespace lab::detail {

// logging with std::format ---------------------------------------------------------------------

template<class... Args>
void log(LogLevel level, std::format_string<Args...> format, Args&&... args) {
  if (level >= log_level()) {
    lab::log(level, std::format(format, std::forward<Args>(args)...));
  }
}

// Gpu ------------------------------------------------------------------------------------------

struct GpuState {
  wgpu::Instance instance;
  wgpu::Adapter adapter;
  wgpu::Device device;
  wgpu::Queue queue;

  std::string label;
  bool throw_on_error = true;
  std::vector<std::string> errors; // every error the device has reported
  size_t errors_thrown = 0;        // how many of them poll() has thrown already

  GpuState();
  ~GpuState();
  GpuState(const GpuState&) = delete;
  GpuState& operator=(const GpuState&) = delete;

  // blocks until the future is done
  void wait(wgpu::Future future) const;

  // processes callbacks and throws the errors that have not been thrown yet
  void poll();
};

// Runs `create` and returns the validation error it caused, if any. Used to turn
// WebGPU errors into exceptions right where an object is created, instead of
// having them reported later without context.
std::optional<std::string> capture_error(const GpuState& gpu, const std::function<void()>& create);

// Copies a texture back from the GPU and writes it to a PNG file (8-bit RGBA and BGRA formats)
void save_texture_png(const GpuState& gpu, const wgpu::Texture& texture, const std::filesystem::path& path,
                      std::string_view label);

// Finds `file` as it is or relative to the directory of the executable, see lab::find_file.
// `label` names the object the file is for, in the error that is thrown if it is not found.
std::filesystem::path locate_file(const std::filesystem::path& file, std::string_view label);

// Window ---------------------------------------------------------------------------------------

struct WindowState {
  GLFWwindow* handle = nullptr;
  bool open = false;

  // input, the "pressed" and "released" sets are cleared by every tick()
  static constexpr size_t key_count = 512;
  static constexpr size_t button_count = 8;
  std::bitset<key_count> keys_down, keys_pressed, keys_released;
  std::bitset<button_count> buttons_down, buttons_pressed, buttons_released;
  Vec2 mouse, mouse_delta, scroll;
  bool mouse_known = false; // false until the first cursor position arrives

  std::function<void(const KeyEvent&)> on_key;
  std::function<void(const MouseButtonEvent&)> on_mouse_button;
  std::function<void(Vec2)> on_scroll;
  std::function<void(Size)> on_resize;

  WindowState() = default;
  ~WindowState();
  WindowState(const WindowState&) = delete;
  WindowState& operator=(const WindowState&) = delete;

  void close();
  void begin_tick(); // forget what happened during the previous tick
};

// Surface --------------------------------------------------------------------------------------

struct SurfaceState {
  std::shared_ptr<GpuState> gpu;
  std::shared_ptr<WindowState> window;
  wgpu::Surface handle;
  wgpu::TextureFormat format = wgpu::TextureFormat::Undefined;
  wgpu::PresentMode present_mode = wgpu::PresentMode::Fifo;
  wgpu::CompositeAlphaMode alpha_mode = wgpu::CompositeAlphaMode::Auto;
  Size configured_size;   // {0, 0} while not configured
  wgpu::Texture texture;  // of the frame being rendered, null between frames
  wgpu::TextureView view; // a view of `texture`
  wgpu::TextureFormat depth_format = wgpu::TextureFormat::Undefined;
  wgpu::Texture depth_texture; // as large as the surface, null without a depth format
  std::string label;

  // Frame capture: with the environment variable LAB_CAPTURE_DIR set, every surface
  // saves one frame as a PNG file into that directory (the frame number can be chosen
  // with LAB_CAPTURE_FRAME, it is 30 by default). Used to check what programs render
  // without looking at them, and to make screenshots.
  std::filesystem::path capture_path; // empty: no capture
  long capture_frame = 0;
  long frames_presented = 0;

  ~SurfaceState();

  Size framebuffer_size() const;
  void configure(Size size);
  wgpu::TextureView current_view();
  void present();
};

// Runtime: what is global by nature -------------------------------------------------------------
// GLFW is one per process, tick() has to reach every window and every Gpu,
// and there is one clock.

struct Runtime {
  static Runtime& get();

  bool glfw_ready = false;
  std::vector<WindowState*> windows;        // open windows
  std::vector<GLFWwindow*> retired_windows; // closed, waiting to be destroyed
  std::vector<GpuState*> gpus;              // all live GPUs

  using Clock = std::chrono::steady_clock;
  Clock::time_point timer_start = Clock::now();
  Clock::time_point last_tick = Clock::now();
  float delta_seconds = 0.0f;

  long exit_after_frames = 0; // from LAB_EXIT_AFTER_FRAMES, 0: never
  int surfaces_created = 0;   // numbers the capture files of programs with several surfaces
  long tick_count = 0;
  bool presented = false; // has a surface been presented since the last tick()?

  void init_glfw();

  // A native window has to outlive the swapchain that presents into it, and the
  // GPU releases swapchains late: when its device is destroyed. A window that is
  // no longer needed is therefore only destroyed once no Gpu is alive.
  void retire_window(GLFWwindow* window);
  void destroy_retired_windows();

  ~Runtime();

private:
  Runtime();
};

} // namespace lab::detail

#endif // WGPU_LAB_DETAIL_H
