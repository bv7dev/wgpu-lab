// The triangle is defined right here, so the program does not need any buffers.
// The index wraps around, which lets a line strip of 4 vertices close the outline.

@vertex
fn vs_main(@builtin(vertex_index) vertex_index: u32) -> @builtin(position) vec4f {
  var positions = array<vec2f, 3>(vec2f(-0.5, -0.5), vec2f(0.5, -0.5), vec2f(0.0, 0.5));
  return vec4f(positions[vertex_index % 3], 0.0, 1.0);
}

@fragment
fn fs_main() -> @location(0) vec4f {
  return vec4f(0.0, 0.4, 1.0, 1.0);
}
