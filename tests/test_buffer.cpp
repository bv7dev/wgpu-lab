#include <lab>

#include <GLFW/glfw3.h>

int main() {
  lab::Webgpu webgpu{"My Instance"};
  lab::Window window{"My Window", 640, 400};
  lab::Shader shader{"My Shader", "shaders/test1.wgsl"};

  lab::Surface surface{window, webgpu};
  lab::Pipeline pipeline{shader, webgpu};

  std::vector<int> message{1, 2, 3, 4, 5, 6, 7, 8};
  lab::ReadableBuffer buffer{webgpu, message};

  auto test = buffer.read_async(2, 3, [](const std::vector<int>& data) {
    std::cout << "callback: ";
    for (auto d : data) {
      std::cout << d << ", ";
    }
    std::cout << std::endl;
  });

  // const void* const* bufmap = buffer.get_read_map(0, len);

  while (lab::tick()) {
    if (test.is_ready()) {
      std::cout << "ready" << std::endl;

      for (auto d : test.data) {
        std::cout << d << ", ";
      }
      std::cout << std::endl;

      test.data.clear();
    }
    // if (*bufmap) {
    //   std::cout << (const char*)(*bufmap) << std::endl;
    //   glfwSetWindowTitle((GLFWwindow*)window.get_handle(),
    //                      (const char*)(*bufmap));
    //   buffer.unmap();
    // }
    pipeline.render_frame(surface);
    lab::sleep(16ms);
  }
}
