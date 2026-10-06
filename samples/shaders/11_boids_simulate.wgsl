// One step of a flock: every boid looks at the boids around it and
//  - moves towards their center (cohesion)
//  - keeps a distance to the closest ones (separation)
//  - flies in their direction (alignment)

struct Boid {
  position: vec2f,
  velocity: vec2f,
};

struct Params {
  delta_time: f32,
  cohesion_distance: f32,
  separation_distance: f32,
  alignment_distance: f32,
  cohesion_scale: f32,
  separation_scale: f32,
  alignment_scale: f32,
};

@group(0) @binding(0) var<uniform> params: Params;
@group(0) @binding(1) var<storage, read> boids_in: array<Boid>;
@group(0) @binding(2) var<storage, read_write> boids_out: array<Boid>;

@compute @workgroup_size(64)
fn main(@builtin(global_invocation_id) id: vec3u) {
  let count = arrayLength(&boids_in);
  let index = id.x;
  if (index >= count) {
    return; // the last workgroup reaches beyond the end of the array
  }

  var position = boids_in[index].position;
  var velocity = boids_in[index].velocity;

  var center = vec2f(0.0);
  var center_count = 0u;
  var away = vec2f(0.0);
  var heading = vec2f(0.0);
  var heading_count = 0u;

  for (var i = 0u; i < count; i++) {
    if (i == index) {
      continue;
    }
    let other = boids_in[i];
    let d = distance(other.position, position);
    if (d < params.cohesion_distance) {
      center += other.position;
      center_count++;
    }
    if (d < params.separation_distance) {
      away -= other.position - position;
    }
    if (d < params.alignment_distance) {
      heading += other.velocity;
      heading_count++;
    }
  }
  if (center_count > 0u) {
    center = center / f32(center_count) - position;
  }
  if (heading_count > 0u) {
    heading /= f32(heading_count);
  }

  velocity += center * params.cohesion_scale + away * params.separation_scale + heading * params.alignment_scale;

  // neither too fast nor standing still
  velocity = normalize(velocity) * clamp(length(velocity), 0.05, 0.4);
  position += velocity * params.delta_time;

  // what leaves on one side comes back in on the other
  position = (position + 3.0) % 2.0 - 1.0;

  boids_out[index] = Boid(position, velocity);
}
