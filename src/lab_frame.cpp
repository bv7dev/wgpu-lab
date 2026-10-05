#include "lab_detail.h"

#include <lab_frame.h>

#include <webgpu/webgpu_cpp_print.h>

#include <algorithm>
#include <limits>
#include <sstream>

namespace lab {

namespace {

std::string describe(const TargetFormat& format) {
  std::ostringstream stream;
  stream << format.color;
  if (format.depth != wgpu::TextureFormat::Undefined) {
    stream << " with depth " << format.depth;
  }
  return stream.str();
}

} // namespace

// Frame -----------------------------------------------------------------------------------------

Frame::Frame(const Gpu& gpu_object) : gpu{gpu_object.state()}, encoder{gpu->device.CreateCommandEncoder()} {}

Frame::Frame(const Surface& surface) : gpu{surface.state()->gpu}, encoder{gpu->device.CreateCommandEncoder()} {}

Frame::~Frame() {
  if (!submitted) {
    try {
      submit();
    } catch (const Error&) {
      // already logged, and a destructor must not throw
    }
  }
}

void Frame::submit() {
  if (submitted) {
    return;
  }
  if (pass_open) {
    detail::fail("frame", "submit: a pass of this frame has not been ended yet");
  }
  submitted = true;

  wgpu::CommandBuffer commands = encoder.Finish();
  gpu->queue.Submit(1, &commands);

  for (const auto& surface : surfaces) {
    surface->present();
  }
  surfaces.clear();
}

// RenderPass ------------------------------------------------------------------------------------

void RenderPass::begin(Frame& target_frame, wgpu::TextureView view, TargetFormat format, const PassOptions& options) {
  frame = &target_frame;
  target = format;
  label = options.label;

  if (!view) {
    return; // nothing to render onto: the pass stays inactive and draw calls do nothing
  }
  if (frame->submitted) {
    detail::fail(label, "the frame of this pass has already been submitted");
  }
  if (frame->pass_open) {
    detail::fail(label, "another pass of the same frame is still open, end it first");
  }

  wgpu::RenderPassColorAttachment attachment{
      .view = view,
      .loadOp = options.clear ? wgpu::LoadOp::Clear : wgpu::LoadOp::Load,
      .storeOp = wgpu::StoreOp::Store,
      .clearValue = options.clear.value_or(wgpu::Color{}),
  };
  wgpu::RenderPassDescriptor desc{
      .label = std::string_view(label),
      .colorAttachmentCount = 1,
      .colorAttachments = &attachment,
  };
  encoder = frame->encoder.BeginRenderPass(&desc);
  frame->pass_open = true;
}

RenderPass::RenderPass(Surface& surface, PassOptions options) {
  own_frame.emplace(surface);
  begin(*own_frame, surface.current_view(), surface, options);
  if (encoder) {
    own_frame->surfaces.push_back(surface.state());
  }
}

RenderPass::RenderPass(Frame& target_frame, Surface& surface, PassOptions options) {
  begin(target_frame, surface.current_view(), surface, options);
  if (encoder && std::ranges::find(frame->surfaces, surface.state()) == frame->surfaces.end()) {
    frame->surfaces.push_back(surface.state());
  }
}

RenderPass::RenderPass(Frame& target_frame, const Texture& texture, PassOptions options) {
  begin(target_frame, texture.view(), texture, options);
}

RenderPass::~RenderPass() {
  try {
    end();
  } catch (const Error&) {
    // already logged, and a destructor must not throw
  }
}

void RenderPass::end() {
  if (encoder) {
    encoder.End();
    encoder = nullptr;
    frame->pass_open = false;
  }
  if (own_frame) {
    own_frame->submit();
  }
}

void RenderPass::draw(const Pipeline& pipeline, const Draw& draw) {
  if (!encoder) {
    return;
  }

  if (pipeline.target() != target) {
    detail::fail(pipeline.label(),
                 std::format("draw: the pipeline was built for the target format {}, but \"{}\" renders to {}. "
                             "A pipeline can only draw to the kind of target it was created for.",
                             describe(pipeline.target()), label, describe(target)));
  }

  const auto& slots = pipeline.slots();
  if (draw.vertex_buffers.size() != slots.size()) {
    detail::fail(pipeline.label(), std::format("draw: the pipeline declares {} vertex buffer(s), but the draw call "
                                               "provides {}",
                                               slots.size(), draw.vertex_buffers.size()));
  }

  // how much the buffers hold, in case the draw call leaves the counts open
  constexpr uint64_t unknown = std::numeric_limits<uint64_t>::max();
  uint64_t vertices_available = unknown;
  uint64_t instances_available = unknown;

  encoder.SetPipeline(pipeline.handle());
  for (uint32_t slot = 0; slot < slots.size(); ++slot) {
    const VertexBufferRef& buffer = draw.vertex_buffers[slot];
    if (buffer.stride != slots[slot].stride) {
      detail::fail(pipeline.label(),
                   std::format("draw: vertex buffer {} has elements of {} bytes, but the pipeline declares {} bytes "
                               "for it: is it the buffer the layout was written for?",
                               slot, buffer.stride, slots[slot].stride));
    }
    uint64_t& available =
        slots[slot].step_mode == wgpu::VertexStepMode::Instance ? instances_available : vertices_available;
    available = std::min(available, buffer.count);
    encoder.SetVertexBuffer(slot, buffer.buffer);
  }
  for (uint32_t group = 0; group < draw.bind_groups.size(); ++group) {
    encoder.SetBindGroup(group, draw.bind_groups[group]);
  }

  const uint32_t instances =
      draw.instances != Draw::all ? draw.instances
      : instances_available == unknown
          ? 1
          : static_cast<uint32_t>(instances_available - std::min<uint64_t>(draw.first_instance, instances_available));

  if (draw.index_buffer.buffer) {
    const IndexBufferRef& indices = draw.index_buffer;
    const uint32_t count = draw.count != Draw::all
                               ? draw.count
                               : static_cast<uint32_t>(indices.count - std::min<uint64_t>(draw.first, indices.count));
    encoder.SetIndexBuffer(indices.buffer, indices.format);
    encoder.DrawIndexed(count, instances, draw.first, draw.base_vertex, draw.first_instance);
  } else {
    if (draw.count == Draw::all && vertices_available == unknown) {
      detail::fail(pipeline.label(), "draw: there is no vertex buffer to take the number of vertices from, "
                                     "set Draw::count");
    }
    const uint32_t count =
        draw.count != Draw::all
            ? draw.count
            : static_cast<uint32_t>(vertices_available - std::min<uint64_t>(draw.first, vertices_available));
    encoder.Draw(count, instances, draw.first, draw.first_instance);
  }
}

void RenderPass::draw(const Pipeline& pipeline, uint32_t vertex_count, uint32_t instance_count) {
  draw(pipeline, Draw{.count = vertex_count, .instances = instance_count});
}

} // namespace lab
