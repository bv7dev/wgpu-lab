#include <lab>

int main() {
  {
    lab::Window first_window{640, 400, "My Window"};
    lab::Window second_window{300, 200, "Test"};

    second_window.set_key_callback([&](int key, int, int, int) {
      if (key == 32) {
        first_window.~Window();
      }
    });

    lab::Webgpu webgpu{};
    webgpu.create_pipeline();

    lab::Surface first_surface{first_window, webgpu};
    lab::Surface second_surface{second_window, webgpu};

    while (lab::tick()) {
      if (first_window.is_open()) first_surface.render_frame();
      if (second_window.is_open()) second_surface.render_frame();
      lab::sleep(16ms);
    }
  }
  {
    lab::Window first_window{640, 400, "My Window"};
    lab::Window second_window{300, 200, "Test"};

    // Todo: Investigate: Maybe 2 webgpu instances could be useful in multi-threaded contexts
    lab::Webgpu webgpu{};
    webgpu.create_pipeline();

    lab::Webgpu webgpu2{};
    webgpu2.create_pipeline();

    lab::Surface first_surface{first_window, webgpu};
    lab::Surface second_surface{second_window, webgpu2};

    while (lab::tick()) {
      if (first_window.is_open()) first_surface.render_frame();
      if (second_window.is_open()) second_surface.render_frame();
      lab::sleep(16ms);
    }
  }
}
