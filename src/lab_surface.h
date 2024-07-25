#ifndef WGPU_LAB_SURFACE_H
#define WGPU_LAB_SURFACE_H

#include "lab_webgpu.h"
#include "lab_window.h"

namespace lab {

struct Surface {
  Window& window;
  Webgpu& webgpu;

  wgpu::Surface wgpu_surface;

  Surface(Window& wnd, Webgpu& wgpu);

  Surface(const Surface&) = delete;
  Surface& operator=(const Surface&) = delete;

  void configure();
  void render_frame();

  ~Surface();
};

} // namespace lab

#endif // WGPU_LAB_SURFACE_H
