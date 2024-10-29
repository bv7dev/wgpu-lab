struct Uniforms {
  ratio: vec2f,
  time: f32,
};

struct VsInput {
  @location(0) vertex_pos: vec2f,
  @location(1) pos: vec2f,
  @location(2) scale: f32,
};

struct VsOutput {
  @builtin(position) position: vec4f,
  @location(0) pos: vec2f,
};

@group(0) @binding(0) var<uniform> uniforms: Uniforms;

@vertex
fn vs_main(in: VsInput) -> VsOutput {
  var out: VsOutput;
  out.position = vec4f((in.pos + in.vertex_pos*in.scale)*uniforms.ratio, 0.0, 1.0);
  out.pos = in.vertex_pos;
	return out;
}

@fragment
fn fs_main(in: VsOutput) -> @location(0) vec4f {
  let intensity = pow(1.0 - length(in.pos), 1.0);
  if (intensity <= 0.0) { return vec4f(0.2, 0.0, 0.0, 0.2); }
  return vec4f(0.2+intensity*0.8);
}
