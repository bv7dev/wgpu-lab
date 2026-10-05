struct Uniforms {
  ratio: vec2f,
  time: f32,
};

struct VSInput {
  @location(0) v_pos: vec2f,
  @location(1) pos_a: vec2f,
  @location(2) pos_b: vec2f,
  @location(3) scale: f32,
};

struct VSOutput {
  @builtin(position) position: vec4f,
  @location(0) modelspace: vec2f,
};

@group(0) @binding(0) var<uniform> uniforms: Uniforms;

@vertex
fn vs_main(@builtin(vertex_index) index: u32, in: VSInput) -> VSOutput {
  var out: VSOutput;

  var ab = in.pos_b - in.pos_a;

  var ang = atan(ab.y/ab.x);
  if (ab.x < 0.0){ ang += 3.14159265; }
  var rot = mat2x2<f32>(cos(ang), -sin(ang), sin(ang), cos(ang));

  var ext: vec2f = vec2f(0.0);
  if (index > 2){ ext.x = length(ab); }

  out.position = vec4f((in.pos_a + (in.v_pos*in.scale+ext)*rot)*uniforms.ratio, 0.0, 1.0);
  out.modelspace = in.v_pos;

	return out;
}

@fragment
fn fs_main(frag: VSOutput) -> @location(0) vec4f {
  let intensity = pow(1.0 - abs(frag.modelspace.y), 2.2);
	return vec4f(intensity);
}
