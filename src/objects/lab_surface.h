#ifndef WGPU_LAB_SURFACE_H
#define WGPU_LAB_SURFACE_H

#include <objects/lab_webgpu.h>
#include <objects/lab_window.h>

namespace lab {

struct Surface {
  Window& window;
  Webgpu& webgpu;

  wgpu::Surface wgpu_surface;

  Surface(Window& wnd, Webgpu& wgpu);

  Surface(const Surface&) = delete;
  Surface& operator=(const Surface&) = delete;

  void reconfigure(int width, int height);
  void reconfigure();

  ~Surface();
};

} // namespace lab

#endif // WGPU_LAB_SURFACE_H
