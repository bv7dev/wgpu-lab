#include <lab>

#include <GLFW/glfw3.h>

int main() {
  lab::Webgpu webgpu{"My Webgpu Instance"};

  lab::Window window_1{"My Window 1", 640, 400};
  lab::Window window_2{"My Window 2", 320, 200};

  lab::Shader shader_1{"My Shader 1", "shaders/test1.wgsl"};
  lab::Shader shader_2{"My Shader 2", "shaders/test2.wgsl"};

  lab::Surface surface_1{window_1, webgpu};
  lab::Surface surface_2{window_2, webgpu};

  lab::Pipeline pipeline_1{shader_1, webgpu};
  lab::Pipeline pipeline_2{shader_2, webgpu};

  bool toggle_pipeline = false;

  window_1.set_key_callback([&toggle_pipeline](const lab::Window::KeyEvent& event) {
    if (event.key == GLFW_KEY_SPACE && event.action == GLFW_PRESS) {
      toggle_pipeline = !toggle_pipeline;
    }
  });

  while (lab::tick()) {
    if (window_1.is_open()) pipeline_1.render_frame(surface_1);

    if (toggle_pipeline) {
      if (window_2.is_open()) pipeline_2.render_frame(surface_2);
    } else {
      if (window_2.is_open()) pipeline_1.render_frame(surface_2);
    }

    lab::sleep(16ms);
  }

  // {
  //   lab::Window first_window{640, 400, "My Window"};
  //   lab::Window second_window{300, 200, "Test"};

  //   second_window.set_key_callback([&](int key, int, int, int) {
  //     if (key == 32) {
  //       first_window.~Window();
  //     }
  //   });

  //   lab::Webgpu webgpu{};
  //   webgpu.create_pipeline();

  //   lab::Surface first_surface{first_window, webgpu};
  //   lab::Surface second_surface{second_window, webgpu};

  //   while (lab::tick()) {
  //     if (first_window.is_open()) first_surface.render_frame();
  //     if (second_window.is_open()) second_surface.render_frame();
  //     lab::sleep(16ms);
  //   }
  // }
  // {
  //   lab::Window first_window{640, 400, "My Window"};
  //   lab::Window second_window{300, 200, "Test"};

  //   // Todo: Investigate: Maybe 2 webgpu instances could be useful in multi-threaded contexts
  //   lab::Webgpu webgpu{};
  //   webgpu.create_pipeline();

  //   lab::Webgpu webgpu2{};
  //   webgpu2.create_pipeline();

  //   lab::Surface first_surface{first_window, webgpu};
  //   lab::Surface second_surface{second_window, webgpu2};

  //   while (lab::tick()) {
  //     if (first_window.is_open()) first_surface.render_frame();
  //     if (second_window.is_open()) second_surface.render_frame();
  //     lab::sleep(16ms);
  //   }
  // }
}
