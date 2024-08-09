#include <lab>

#include <memory>

#include <GLFW/glfw3.h>

int main() {

  // TEST 1 - press space to change pipeline of window 2
  {
    lab::Webgpu webgpu{"My Webgpu Instance"};

    lab::Window window_2{"My Window 2", 320, 200};
    lab::Window window_1{"My Window 1   Press Space to toggle My Window 2 pipeline", 640, 400};

    lab::Shader shader_1{"My Shader 1", "shaders/test1.wgsl"};
    lab::Shader shader_2{"My Shader 2", "shaders/test2.wgsl"};

    lab::Surface surface_1{window_1, webgpu};
    lab::Surface surface_2{window_2, webgpu};

    lab::Pipeline pipeline_1{shader_1, webgpu};
    lab::Pipeline pipeline_2{shader_2, webgpu};

    bool toggle_pipeline = false;

    window_1.set_key_callback([&toggle_pipeline](const lab::KeyEvent& event) {
      if (event.key == lab::KeyCode::space && event.action == lab::InputType::press) {
        toggle_pipeline = !toggle_pipeline;
      }
    });

    while (lab::tick()) {
      if (window_1.is_open()) pipeline_1.render_frame(surface_1, {3, 1});

      if (toggle_pipeline) {
        if (window_2.is_open()) pipeline_2.render_frame(surface_2, {3, 1});
      } else {
        if (window_2.is_open()) pipeline_1.render_frame(surface_2, {3, 1});
      }

      lab::sleep(16ms);
    }
  }

  // TEST 2 - press space to open second window
  {
    using namespace lab;

    Webgpu webgpu{"My Instance"};
    Window window{"My Window", 640, 400};
    Shader shader{"My Shader", "shaders/test1.wgsl"};

    Surface surface{window, webgpu};
    Pipeline pipeline{shader, webgpu};

    std::unique_ptr<Window> sometimes_open;
    std::unique_ptr<Surface> sometimes_surf;

    window.set_key_callback([&](const KeyEvent& event) {
      if (event.key == KeyCode::space && event.action == InputType::press) {
        if (!sometimes_open) {
          sometimes_open = std::make_unique<Window>("test", 400, 300);
          sometimes_surf = std::make_unique<Surface>(*sometimes_open.get(), webgpu);

          sometimes_open.get()->set_key_callback([&](const KeyEvent& event) {
            if (event.key == KeyCode::tab && event.action == InputType::release) {
              sometimes_surf.get()->~Surface();
              sometimes_open.get()->~Window();
              sometimes_surf.release();
              sometimes_open.release();
            }
          });
        }
      }
    });

    while (tick()) {
      pipeline.render_frame(surface, {3, 1});

      if (sometimes_open && sometimes_open.get()->is_open()) {
        pipeline.render_frame(*sometimes_surf.get(), {3, 1});
      }

      sleep(16ms);
    }
  }

  // TEST 3 - press space to close the first window
  {
    lab::Window first_window{"My Window", 640, 400};
    lab::Window second_window{"Press space to close My Window", 500, 200};

    second_window.set_key_callback([&](const lab::KeyEvent& event) {
      if (event.key == lab::KeyCode::space) {
        first_window.~Window();
      }
    });

    lab::Webgpu webgpu{"My instance"};
    lab::Shader shader{"My shader", "shaders/test1.wgsl"};
    lab::Pipeline pipeline{shader, webgpu};

    lab::Surface first_surface{first_window, webgpu};
    lab::Surface second_surface{second_window, webgpu};

    while (lab::tick()) {
      if (first_window.is_open()) pipeline.render_frame(first_surface, {3, 1});
      if (second_window.is_open()) pipeline.render_frame(second_surface, {3, 1});
      lab::sleep(16ms);
    }
  }

  // TEST 4 - test multiple webgpu instances
  {
    lab::Window first_window{"uses Webgpu instance 1", 640, 400};
    lab::Window second_window{"uses Webgpu instance 2", 300, 200};
    lab::Shader shader{"My Shader", "shaders/test2.wgsl"};

    // Todo: Investigate:
    // Maybe 2 webgpu instances could be useful in multi-threaded contexts
    lab::Webgpu webgpu1{"Inst 1"};
    lab::Webgpu webgpu2{"Inst 2"};

    lab::Pipeline pipe1{shader, webgpu1};
    lab::Pipeline pipe2{shader, webgpu2};

    lab::Surface first_surface{first_window, webgpu1};
    lab::Surface second_surface{second_window, webgpu2};

    while (lab::tick()) {
      if (first_window.is_open()) pipe1.render_frame(first_surface, {3, 1});
      if (second_window.is_open()) pipe2.render_frame(second_surface, {3, 1});
      lab::sleep(16ms);
    }
  }

  // TEST 7 - unholy experiments
  {
    // time experiments
    using namespace lab;

    Webgpu webgpu{"My Instance"};
    Window window{"My Window", 640, 400};
    Shader shader{"My Shader", "shaders/test1.wgsl"};

    Surface surface{window, webgpu};
    Pipeline pipeline{shader, webgpu};

    window.set_key_callback([&window](const KeyEvent& event) {
      if (event.key == KeyCode::space && event.action == InputType::press) {
        window.clear_resize_callback();
      }
    });

    Window w2{"bla", 400, 280};
    Surface s2{w2, webgpu};

    int frame = 0;
    double last_t = glfwGetTime();
    double last_T = last_t;

    while (tick()) {
      if (window.is_open()) pipeline.render_frame(surface, {3, 1});
      if (w2.is_open()) pipeline.render_frame(s2, {3, 1});
      frame++;

      if ((frame & 0xFF) == 0x00) {
        std::cout << "fps: " << (256.0 / (glfwGetTime() - last_t)) << std::endl;
        last_t = glfwGetTime();
      }

      double x = glfwGetTime() - last_T;
      last_T = glfwGetTime();
      if ((frame & 0xFF) == 0x01) {
        std::cout << x << std::endl;
      }

      sleep(std::chrono::duration<double, std::ratio<1>>(0.016 - x));
    }
  }
}
