struct VsOutput {
  @builtin(position) position: vec4f,
  @location(0) color: vec3f,
};

@vertex
fn vs_main(
  @location(0) corner: vec2f,   // per vertex: the shape of a boid
  @location(1) position: vec2f, // per instance: straight from the buffer the simulation writes
  @location(2) velocity: vec2f,
) -> VsOutput {
  // turn the shape so that it points where the boid is flying
  let angle = -atan2(velocity.x, velocity.y);
  let turned = vec2f(corner.x * cos(angle) - corner.y * sin(angle), corner.x * sin(angle) + corner.y * cos(angle));

  var out: VsOutput;
  out.position = vec4f(turned + position, 0.0, 1.0);
  // the color tells the direction
  out.color = 0.55 + 0.45 * vec3f(sin(angle), sin(angle + 2.1), sin(angle + 4.2));
  return out;
}

@fragment
fn fs_main(in: VsOutput) -> @location(0) vec4f {
  return vec4f(in.color, 1.0);
}
