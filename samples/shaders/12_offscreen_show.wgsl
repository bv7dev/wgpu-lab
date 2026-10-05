// Shows a texture across the whole target

@group(0) @binding(0) var image: texture_2d<f32>;
@group(0) @binding(1) var image_sampler: sampler;

struct VsOutput {
  @builtin(position) position: vec4f,
  @location(0) uv: vec2f,
};

@vertex
fn vs_main(@builtin(vertex_index) vertex_index: u32) -> VsOutput {
  // one triangle that is large enough to cover the target
  var corners = array<vec2f, 3>(vec2f(-1.0, -1.0), vec2f(3.0, -1.0), vec2f(-1.0, 3.0));
  let corner = corners[vertex_index];

  var out: VsOutput;
  out.position = vec4f(corner, 0.0, 1.0);
  out.uv = vec2f(corner.x * 0.5 + 0.5, 0.5 - corner.y * 0.5); // textures count their rows from the top
  return out;
}

@fragment
fn fs_main(in: VsOutput) -> @location(0) vec4f {
  return textureSample(image, image_sampler, in.uv);
}
