@vertex
fn vs_main(@location(0) in_vertex_position: vec2f, @location(1) instance_pos: vec2f) -> @builtin(position) vec4f {
	return vec4f((instance_pos + in_vertex_position*1.0)*0.2, 0.0, 1.0);
}

@fragment
fn fs_main() -> @location(0) vec4f {
	return vec4f(0.0, 0.4, 1.0, 1.0);
}

