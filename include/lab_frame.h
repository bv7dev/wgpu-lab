#ifndef WGPU_LAB_FRAME_H
#define WGPU_LAB_FRAME_H

#include <lab_gpu.h>
#include <lab_pipeline.h>
#include <lab_surface.h>
#include <lab_texture.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace lab {

// A batch of work for the GPU: one or more passes that are submitted together.
// Destroying the frame submits it and presents every surface that was rendered to.
// ```cpp
// {
//   lab::Frame frame(gpu);
//   {
//     lab::RenderPass shadows(frame, shadow_map);
//     shadows.draw(shadow_pipeline, scene);
//   }
//   lab::RenderPass main(frame, surface);
//   main.draw(pipeline, scene);
// } // main ends, the frame is submitted, the surface is presented
// ```
// A frame with a single pass onto a surface does not need a Frame object, see `lab::RenderPass`.
class Frame {
public:
  explicit Frame(const Gpu& gpu);
  explicit Frame(const Surface& surface); // a frame on the Gpu of this surface

  Frame(const Frame&) = delete;
  Frame& operator=(const Frame&) = delete;

  // submits the frame if `submit()` was not called
  ~Frame();

  // Hands the recorded passes to the GPU and presents the surfaces that were rendered to
  //  - all passes of the frame have to be ended before
  void submit();

  const wgpu::CommandEncoder& handle() const { return encoder; }

private:
  friend class RenderPass;
  friend class ComputePass;

  std::shared_ptr<detail::GpuState> gpu;
  wgpu::CommandEncoder encoder;
  std::vector<std::shared_ptr<detail::SurfaceState>> surfaces; // to present after submitting
  bool pass_open = false;
  bool submitted = false;
};

struct PassOptions {
  // color the target is cleared to before drawing, nullopt keeps what is in it
  std::optional<wgpu::Color> clear = wgpu::Color{0.08, 0.08, 0.085, 1.0};

  // A depth buffer for passes into a texture: a texture with a depth format and the size
  // of the target. A surface brings its own, see SurfaceOptions::depth.
  const Texture* depth = nullptr;

  // depth the depth buffer is cleared to, nullopt keeps what is in it
  std::optional<float> clear_depth = 1.0f;

  std::string label = "lab render pass";
};

// A sequence of draw calls into one target. Destroying the pass ends it.
// ```cpp
// while (lab::tick()) {
//   lab::RenderPass pass(surface);       // one frame with one pass
//   pass.draw(node_pipeline, nodes);
//   pass.draw(edge_pipeline, edges);
// }                                      // submitted and presented here
// ```
class RenderPass {
public:
  // A pass that is a whole frame by itself: ending it submits and presents
  explicit RenderPass(Surface& surface, PassOptions options = {});

  // A pass within a frame, onto a surface or into a texture
  RenderPass(Frame& frame, Surface& surface, PassOptions options = {});
  RenderPass(Frame& frame, const Texture& target, PassOptions options = {});

  RenderPass(const RenderPass&) = delete;
  RenderPass& operator=(const RenderPass&) = delete;

  // ends the pass if `end()` was not called
  ~RenderPass();

  // Draws with a pipeline. Throws lab::Error if the pipeline was built for
  // another target format or if the buffers do not fit its vertex layouts.
  void draw(const Pipeline& pipeline, const Draw& draw);
  // for pipelines without vertex buffers
  void draw(const Pipeline& pipeline, uint32_t vertex_count, uint32_t instance_count = 1);

  void end();

  // False if there is nothing to render onto this time (see Surface::current_view).
  // Draw calls on such a pass do nothing, so checking is only needed before using `handle()`.
  explicit operator bool() const { return encoder != nullptr; }

  // for everything the lab does not wrap: `pass.handle().SetViewport(...)`
  const wgpu::RenderPassEncoder& handle() const { return encoder; }

private:
  void begin(Frame& frame, wgpu::TextureView view, wgpu::TextureView depth_view, TargetFormat format,
             const PassOptions& options);

  std::optional<Frame> own_frame;
  Frame* frame = nullptr;
  wgpu::RenderPassEncoder encoder;
  TargetFormat target{wgpu::TextureFormat::Undefined};
  std::string label;
};

} // namespace lab

#endif // WGPU_LAB_FRAME_H
