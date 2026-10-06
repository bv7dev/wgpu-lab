#ifndef WGPU_LAB_SURFACE_H
#define WGPU_LAB_SURFACE_H

#include <lab_gpu.h>
#include <lab_window.h>

namespace lab {

namespace detail {
struct SurfaceState;
}

struct SurfaceOptions {
  // `Fifo` waits for the display (vsync), `Immediate` and `Mailbox` do not
  wgpu::PresentMode present_mode = wgpu::PresentMode::Fifo;

  // `Undefined` picks BGRA8Unorm or RGBA8Unorm, whichever the window system prefers.
  // Other formats it supports can be asked for, e.g. RGBA16Float for HDR output.
  wgpu::TextureFormat format = wgpu::TextureFormat::Undefined;

  // A depth format (e.g. Depth24Plus) gives the surface a depth buffer, which it keeps
  // at the size of the window. Render passes onto the surface then test against it.
  wgpu::TextureFormat depth = wgpu::TextureFormat::Undefined;
};

// What a Gpu renders onto to make it appear in a Window.
// The surface follows the size of its window on its own.
// ```cpp
// lab::Surface surface(gpu, window);
// while (lab::tick()) {
//   lab::RenderPass pass(surface);
//   pass.draw(pipeline, 3);
// }
// ```
class Surface {
public:
  Surface(Gpu& gpu, Window& window, SurfaceOptions options = {});

  Surface(Surface&&) = default;
  Surface& operator=(Surface&&) = default;

  wgpu::TextureFormat format() const;
  wgpu::TextureFormat depth_format() const; // Undefined if the surface has no depth buffer
  Size size() const;                        // in pixels
  float aspect() const;                     // width divided by height

  // The texture view to render the current frame into
  //  - stays the same until `present()` is called
  //  - is null if there is nothing to render onto at the moment (the window is
  //    minimized or the surface has to adapt to a new size first): skip the frame
  //  - `lab::RenderPass` and `lab::Frame` call this and `present()` for you
  wgpu::TextureView current_view();

  // The depth buffer that goes with `current_view()`, null if the surface has none
  wgpu::TextureView depth_view();

  // Shows what has been rendered into `current_view()`
  void present();

  const wgpu::Surface& handle() const;

  // internal
  const std::shared_ptr<detail::SurfaceState>& state() const { return shared_state; }

private:
  std::shared_ptr<detail::SurfaceState> shared_state;
};

} // namespace lab

#endif // WGPU_LAB_SURFACE_H
