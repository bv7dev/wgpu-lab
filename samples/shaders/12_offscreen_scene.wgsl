// The scene that is rendered into the small texture: a spinning triangle

@group(0) @binding(0) var<uniform> time: f32;

struct VsOutput {
  @builtin(position) position: vec4f,
  @location(0) color: vec3f,
};

@vertex
fn vs_main(@builtin(vertex_index) vertex_index: u32) -> VsOutput {
  var corners = array<vec2f, 3>(vec2f(0.0, 0.8), vec2f(-0.7, -0.4), vec2f(0.7, -0.4));
  var colors = array<vec3f, 3>(vec3f(1.0, 0.3, 0.2), vec3f(0.2, 1.0, 0.4), vec3f(0.3, 0.4, 1.0));

  let corner = corners[vertex_index];
  let turned = vec2f(corner.x * cos(time) - corner.y * sin(time), corner.x * sin(time) + corner.y * cos(time));

  var out: VsOutput;
  out.position = vec4f(turned * vec2f(0.625, 1.0), 0.0, 1.0); // the texture is 96 x 60 pixels
  out.color = colors[vertex_index];
  return out;
}

@fragment
fn fs_main(in: VsOutput) -> @location(0) vec4f {
  return vec4f(in.color, 1.0);
}
