struct Uniforms {
  ratio: vec2f,
  time: f32,
  scale: f32,
  selected: u32, // index of the instance the arrow keys move
};

struct VertexInput {
  @location(0) position: vec2f,          // per vertex: the same for every triangle
  @location(1) instance_position: vec2f, // per instance: where this triangle is
};

struct VertexOutput {
  @builtin(position) position: vec4f,
  @location(0) color: vec3f,
};

@group(0) @binding(0) var<uniform> uniforms: Uniforms;

@vertex
fn vs_main(in: VertexInput, @builtin(instance_index) instance: u32) -> VertexOutput {
  // every instance spins with its own phase
  let angle = uniforms.time + f32(instance) * 0.3;
  let rotated = in.position * mat2x2<f32>(cos(angle), -sin(angle), sin(angle), cos(angle));

  var out: VertexOutput;
  out.position = vec4f((in.instance_position + rotated * uniforms.scale) * uniforms.ratio, 0.0, 1.0);

  // a color per instance, the selected one is bright
  let hue = f32(instance) * 0.9;
  out.color = 0.25 + 0.2 * vec3f(sin(hue), sin(hue + 2.1), sin(hue + 4.2));
  if (instance == uniforms.selected) {
    out.color = vec3f(1.0, 0.9, 0.4);
  }
  return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
  return vec4f(in.color, 1.0);
}
