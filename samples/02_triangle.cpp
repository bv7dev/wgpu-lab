// 02: the smallest program that renders something

#include <lab>

int main() {
  lab::Gpu gpu; // the connection to the graphics card
  lab::Window window("hold space for the outline", 640, 400);
  lab::Surface surface(gpu, window); // lets the gpu render into the window

  lab::Shader shader(gpu, "shaders/02_triangle.wgsl");

  // A pipeline is a shader plus everything that is fixed while drawing with it.
  // It is made for one kind of render target, here the surface.
  lab::Pipeline filled(gpu, shader, {.target = surface});

  // the same shader with another setting is another pipeline
  lab::Pipeline outline(gpu, shader, {.target = surface, .topology = wgpu::PrimitiveTopology::LineStrip});

  while (lab::tick()) {
    if (window.key_pressed(lab::KeyCode::escape)) {
      window.close();
    }

    // render_frame() draws one frame with one draw call: 3 or 4 vertices here
    if (window.key(lab::KeyCode::space)) {
      outline.render_frame(surface, 4);
    } else {
      filled.render_frame(surface, 3);
    }
  }
}
