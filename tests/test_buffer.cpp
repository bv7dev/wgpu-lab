#include <lab>

#include <GLFW/glfw3.h>

int main() {
  lab::Webgpu webgpu{"My Instance"};
  lab::Window window{"My Window", 640, 400};
  lab::Shader shader{"My Shader", "shaders/test1.wgsl"};

  lab::Surface surface{window, webgpu};
  lab::Pipeline pipeline{shader, webgpu};

  lab::Buffer testbuf{1024, webgpu};

  const size_t len = 20;
  const uint8_t msg[len] = "This is a Warning!\n";

  testbuf.write(0, len, msg);

  // lab::WriteMap& wmap = testbuf.get_write_map();
  // for (int i = 0; i < len; ++i) {
  //   reinterpret_cast<uint8_t*>(wmap.data)[i] = msg[i];
  // }
  // testbuf.unmap();

  const lab::ReadMap& bufmap = testbuf.get_read_map();

  while (lab::tick()) {
    if (bufmap.data) {
      std::cout << (const char*)bufmap.data << std::endl;
      glfwSetWindowTitle((GLFWwindow*)window.get_handle(),
                         (const char*)bufmap.data);
      testbuf.unmap();
    }
    pipeline.render_frame(surface);
    lab::sleep(16ms);
  }
}
