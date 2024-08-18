struct Uniforms {
  ratio: vec2f,
  time: f32,
  scale: f32,
};

struct VertexInput {
  @location(0) position: vec2f,
};

struct VertexOutput {
  @builtin(position) position: vec4f,
};

@group(0) @binding(0) var<uniform> uniforms: Uniforms;
@group(0) @binding(1) var gradientTexture: texture_2d<f32>;

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
  var out: VertexOutput;
  out.position =  vec4f(
      (vec2f(cos(uniforms.time), sin(uniforms.time)) +
       vec2f(in.position))*uniforms.ratio*uniforms.scale, 0.0, 1.0);
	return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
  let color = textureLoad(gradientTexture, vec2i(in.position.xy), 0).rgb;
	return vec4f(color*(sin(uniforms.time)+1.0), 1.0);
}
