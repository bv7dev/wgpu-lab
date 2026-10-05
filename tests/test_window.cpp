#include "common.h"

#include <memory>

// These tests open windows, so they need a display. They are labelled "display"
// in CTest: `ctest -LE display` leaves them out.

static lab::Pipeline make_triangle_pipeline(lab::Gpu& gpu, const lab::Surface& surface) {
  auto shader = lab::Shader::from_source(
      gpu, fullscreen_vertex_shader + "@fragment fn fs_main() -> @location(0) vec4f { return vec4f(1.0); }");
  return lab::Pipeline(gpu, shader, {.target = surface});
}

TEST_CASE("window: render_frame draws onto a surface") {
  lab::Gpu gpu;
  lab::Window window("test", 200, 150);
  lab::Surface surface(gpu, window);
  lab::Pipeline pipeline = make_triangle_pipeline(gpu, surface);

  CHECK(surface.size() == window.framebuffer_size());
  CHECK(surface.aspect() == doctest::Approx(200.0 / 150.0).epsilon(0.02));

  int frames = 0;
  for (int i = 0; i < 5 && lab::tick(); ++i) {
    frames += pipeline.render_frame(surface, 3);
  }
  CHECK(frames == 5);
  CHECK(gpu.errors().empty());
}

TEST_CASE("window: tick returns false once all windows are closed") {
  lab::Window first("first", 200, 150);
  lab::Window second("second", 200, 150);
  CHECK(lab::tick());

  first.close();
  CHECK_FALSE(first.is_open());
  CHECK(lab::tick());

  second.close();
  CHECK_FALSE(lab::tick());
}

// The native window has to outlive the swapchain the gpu keeps for it, whatever
// the order in which a program lets go of its objects.
TEST_CASE("window: gpu, window and surface can be destroyed in any order") {
  auto gpu = std::make_unique<lab::Gpu>();
  auto window = std::make_unique<lab::Window>("test", 200, 150);
  auto surface = std::make_unique<lab::Surface>(*gpu, *window);
  {
    lab::Pipeline pipeline = make_triangle_pipeline(*gpu, *surface);
    REQUIRE(lab::tick());
    REQUIRE(pipeline.render_frame(*surface, 3));
  }

  SUBCASE("surface, window, gpu") {
    surface.reset();
    window.reset();
    gpu.reset();
  }
  SUBCASE("gpu, surface, window") {
    gpu.reset();
    surface.reset();
    window.reset();
  }
  SUBCASE("window, surface, gpu") {
    window.reset();
    surface.reset();
    gpu.reset();
  }
  SUBCASE("window, gpu, surface") {
    window.reset();
    gpu.reset();
    surface.reset();
  }
  SUBCASE("gpu, window, surface") {
    gpu.reset();
    window.reset();
    surface.reset();
  }
  SUBCASE("surface, gpu, window") {
    surface.reset();
    gpu.reset();
    window.reset();
  }
}

TEST_CASE("window: a surface keeps working after its window was closed and moved") {
  lab::Gpu gpu;
  lab::Window window("test", 200, 150);
  lab::Surface surface(gpu, window);
  lab::Pipeline pipeline = make_triangle_pipeline(gpu, surface);

  // lab objects are handles: moving them does not disturb what is going on
  lab::Window moved_window = std::move(window);
  lab::Surface moved_surface = std::move(surface);
  CHECK(lab::tick());
  CHECK(pipeline.render_frame(moved_surface, 3));

  moved_window.close();
  CHECK_FALSE(lab::tick());
  CHECK_NOTHROW(pipeline.render_frame(moved_surface, 3)); // rendering into a hidden window is harmless
  CHECK(gpu.errors().empty());
}

TEST_CASE("window: keyboard and mouse state start out empty") {
  lab::Window window("test", 200, 150);
  REQUIRE(lab::tick());

  CHECK_FALSE(window.key(lab::KeyCode::space));
  CHECK_FALSE(window.key_pressed(lab::KeyCode::space));
  CHECK_FALSE(window.key(lab::KeyCode::unknown)); // out of range, must not crash
  CHECK_FALSE(window.mouse_button(lab::MouseButton::left));
  CHECK(window.scroll().y == 0.0f);
}
