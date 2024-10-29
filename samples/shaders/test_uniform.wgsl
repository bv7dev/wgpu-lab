@group(0) @binding(0) var<uniform> time: f32;
@group(1) @binding(0) var<uniform> x: f32;
@group(1) @binding(1) var<uniform> y: f32;

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
  var pos: vec2f = vec2f(in.position) + vec2f(x, 0.0) + vec2f(cos(time), sin(time))*0.5;
  out.position = vec4f(pos, 0.0, 1.0);
  out.color = vec3f(in.grayscale);
	return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
  var color: vec3f = in.color + vec3f(0.0, sin(y), 0.0);
	return vec4f(color, 1.0);
}
