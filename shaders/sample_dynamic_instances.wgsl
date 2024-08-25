struct Uniforms {
  ratio: vec2f,
  time: f32,
  scale: f32,
};

struct VertexInput {
  @location(0) position: vec2f,
  @location(1) uv: vec2f,
  @location(2) instance_position: vec2f,
};

struct VertexOutput {
  @builtin(position) position: vec4f,
  @location(0) uv: vec2f,
  @location(1) inst_shift: f32,
};

@group(0) @binding(0) var<uniform> uniforms: Uniforms;
@group(0) @binding(1) var gradientTexture: texture_2d<f32>;

@vertex
fn vs_main(in: VertexInput, @builtin(vertex_index) vert_id: u32, @builtin(instance_index) inst_id: u32) -> VertexOutput {
  var t: f32 = uniforms.time;
  var cos_t: f32 = cos(t + f32(inst_id)*0.3);
  var sin_t: f32 = sin(t + f32(inst_id)*0.3);

  var vertex_rotated: vec2f = in.position*mat2x2<f32>(cos_t, -sin_t, sin_t, cos_t);

  var out: VertexOutput;
  out.position = vec4f(in.instance_position+(vertex_rotated*uniforms.scale +
                       vec2f(cos_t, sin_t)*uniforms.scale*0.5) *
                       uniforms.ratio, 0.0, 1.0);

  // shift some vertices to change per instance texture appearance
  var inst_uv: vec2f = in.uv;
  if (vert_id == 0) {
    inst_uv.x += f32(inst_id)*0.2;
  }
  if (vert_id == 2) {
    inst_uv.y -= f32(inst_id)*0.1;
  }
  out.uv = inst_uv;

  out.inst_shift = f32(inst_id)*0.7 + f32(vert_id)*0.4;

	return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
  let color: vec3f = textureLoad(gradientTexture, vec2i(in.uv*256), 0).rgb;
	return vec4f(color*((sin(uniforms.time+in.inst_shift)+1.0)*2.5), 1.0);
}
