struct Scene {
  view_projection: mat4x4f,
  light_direction: vec4f,
};

@group(0) @binding(0) var<uniform> scene: Scene;
@group(0) @binding(1) var ground_texture: texture_2d<f32>;
@group(0) @binding(2) var ground_sampler: sampler;

struct VsOutput {
  @builtin(position) position: vec4f,
  @location(0) uv: vec2f,
};

@vertex
fn vs_main(@location(0) position: vec3f, @location(1) uv: vec2f) -> VsOutput {
  var out: VsOutput;
  out.position = scene.view_projection * vec4f(position, 1.0);
  out.uv = uv;
  return out;
}

@fragment
fn fs_main(in: VsOutput) -> @location(0) vec4f {
  // uv runs beyond 1, the sampler repeats the texture
  let color = textureSample(ground_texture, ground_sampler, in.uv).rgb;
  let light = max(dot(vec3f(0.0, 1.0, 0.0), -scene.light_direction.xyz), 0.0);
  return vec4f(color * (0.35 + 0.65 * light), 1.0);
}
