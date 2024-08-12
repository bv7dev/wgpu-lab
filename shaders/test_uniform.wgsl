@group(0) @binding(0) var<uniform> time: f32;

struct VertexInput {
  @location(0) position: vec2f,
  @location(1) grayscale: f32,
};

struct VertexOutput {
  @builtin(position) position: vec4f,
  @location(0) color: vec3f,
};

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
  var out: VertexOutput;
  var pos: vec2f = vec2f(in.position) + vec2f(cos(time), sin(time))*0.5;
  out.position = vec4f(pos, 0.0, 1.0);
  out.color = vec3f(in.grayscale);
	return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
	return vec4f(in.color, 1.0);
}
