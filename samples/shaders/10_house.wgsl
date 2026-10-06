struct Scene {
  view_projection: mat4x4f,
  light_direction: vec4f, // xyz: the direction the light travels in
};

@group(0) @binding(0) var<uniform> scene: Scene;

struct VsOutput {
  @builtin(position) position: vec4f,
  @location(0) normal: vec3f,
  @location(1) height: f32,
};

@vertex
fn vs_main(@location(0) position: vec3f, @location(1) normal: vec3f) -> VsOutput {
  var out: VsOutput;
  out.position = scene.view_projection * vec4f(position, 1.0);
  out.normal = normal;
  out.height = position.y;
  return out;
}

@fragment
fn fs_main(in: VsOutput) -> @location(0) vec4f {
  let normal = normalize(in.normal);

  // the model has no materials: surfaces above the walls that face upwards are the roof
  let walls = vec3f(0.92, 0.88, 0.78);
  let roof = vec3f(0.70, 0.27, 0.20);
  let is_roof = step(1.05, in.height) * step(0.2, normal.y);
  let base = mix(walls, roof, is_roof);

  // surfaces that face the light are bright, the rest keeps some ambient light
  let light = max(dot(normal, -scene.light_direction.xyz), 0.0);
  return vec4f(base * (0.35 + 0.65 * light), 1.0);
}
