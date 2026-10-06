// 09: one Gpu renders into several windows
//     space opens another window, closing the last window ends the program

#include <lab>

#include <format>
#include <vector>

struct Color {
  float r, g, b, a;
};

// what belongs to one window: lab objects can be moved, so they can live in a vector
struct View {
  lab::Window window;
  lab::Surface surface;
  lab::Buffer<Color> color;
  wgpu::BindGroup bind_group = nullptr;
};

int main() {
  lab::Gpu gpu;
  std::vector<View> views;

  auto open_window = [&] {
    const int number = static_cast<int>(views.size()) + 1;
    const Color color{0.3f * float(number % 3), 0.25f * float(number % 4), 0.8f - 0.15f * float(number % 5), 1.0f};

    lab::Window window(std::format("window {} - press space for another one", number), 480, 300);
    lab::Surface surface(gpu, window); // every window needs its own surface
    lab::Buffer<Color> color_buffer(gpu, {color}, wgpu::BufferUsage::Uniform);
    views.push_back({std::move(window), std::move(surface), std::move(color_buffer)});
  };
  open_window();
  open_window();

  // all surfaces of a gpu on the same desktop have the same format, so one pipeline serves them all
  lab::Shader shader(gpu, "shaders/09_multi_window.wgsl");
  lab::Pipeline pipeline(gpu, shader, {.target = views.front().surface});

  while (lab::tick()) {
    bool space_pressed = false;
    for (View& view : views) {
      space_pressed = space_pressed || view.window.key_pressed(lab::KeyCode::space);
    }
    if (space_pressed) {
      open_window();
    }

    // forget the windows that were closed, this destroys their surfaces and buffers as well
    std::erase_if(views, [](const View& view) { return !view.window.is_open(); });

    // one frame for all windows: every window gets a render pass,
    // and all of them are shown when the frame goes out of scope
    lab::Frame frame(gpu);
    for (View& view : views) {
      if (!view.bind_group) {
        view.bind_group = pipeline.bind_group(0, {{0, view.color}});
      }
      lab::RenderPass pass(frame, view.surface);
      pass.draw(pipeline, {.bind_groups = {view.bind_group}, .count = 3});
    }
  }
}
