@group(0) @binding(0) var<uniform> color: vec4f;

@vertex
fn vs_main(@builtin(vertex_index) vertex_index: u32) -> @builtin(position) vec4f {
  var positions = array<vec2f, 3>(vec2f(-0.7, -0.5), vec2f(0.4, -0.2), vec2f(0.0, 0.8));
  return vec4f(positions[vertex_index], 0.0, 1.0);
}

@fragment
fn fs_main() -> @location(0) vec4f {
  return color;
}
