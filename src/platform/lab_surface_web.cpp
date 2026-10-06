// The web: the program runs in a browser, the window is an HTML canvas and WebGPU
// is the browser's own. Dawn's Emscripten port (emdawnwebgpu) provides webgpu_cpp.h
// on top of it, the contrib.glfw3 port provides GLFW.

#include "platform/lab_platform.h"

#include <GLFW/emscripten_glfw3.h>
#include <GLFW/glfw3.h>
#include <emscripten/emscripten.h>

namespace lab::platform {

wgpu::Surface create_surface(const wgpu::Instance& instance, GLFWwindow*) {
  // the canvas the page's Module.canvas points at, which is the one GLFW draws into
  wgpu::EmscriptenSurfaceSourceCanvasHTMLSelector source;
  source.selector = "#canvas";
  wgpu::SurfaceDescriptor descriptor{.nextInChain = &source, .label = "lab surface"};
  return instance.CreateSurface(&descriptor);
}

void window_created(GLFWwindow* window) {
  // the canvas fills the browser window and follows its size
  emscripten::glfw3::MakeCanvasResizable(window, "window");
}

// Gives control back to the browser until it is about to draw the next frame.
// This is what makes `while (lab::tick())` work on the web: the loop sleeps between
// iterations instead of blocking the page (it needs ASYNCIFY, see samples/CMakeLists.txt).
// (no arrow function in here: the C preprocessor would turn `=>` into `= >`)
EM_ASYNC_JS(void, lab_await_next_frame, (),
            { await new Promise(function(resolve) { requestAnimationFrame(resolve); }); });

void present(const wgpu::Surface&) {} // the browser shows the canvas at the next animation frame

void end_of_tick() { lab_await_next_frame(); }

std::filesystem::path executable_path() { return {}; }

std::filesystem::path executable_directory() {
  std::error_code ignored;
  return std::filesystem::current_path(ignored);
}

} // namespace lab::platform
