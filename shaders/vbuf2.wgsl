struct VertexInput {
  @location(0) position: vec2f,
  @location(1) grayscale: f32,
};

struct VertexOutput {
  @builtin(position) position: vec4f,
  @location(0) color: f32,
};

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
  var out: VertexOutput;
  out.position = vec4f(in.position, 0.0, 1.0);
  out.color = in.grayscale;
	return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
	return vec4f(vec3f(in.color), 1.0);
}
