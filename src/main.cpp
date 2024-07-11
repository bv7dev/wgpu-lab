#include <iostream>

#include <GLFW/glfw3.h>
#include <webgpu/webgpu.h>

int main(int argc, char** argv) {
  std::cout << "Hi!" << std::endl;
  for (int i = 0; i < argc; ++i)
    std::cout << argv[i] << ' ';
  std::cout << std::endl;

  if (!glfwInit()) return 1;
  GLFWwindow* window = glfwCreateWindow(640, 400, "Hi!", NULL, NULL);
  if (!window) {
    glfwTerminate();
    return 1;
  }

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
  }

  WGPUInstanceDescriptor desc = {};
  desc.nextInChain = nullptr;

  WGPUInstance instance = wgpuCreateInstance(&desc);
  if (!instance) {
    std::cerr << "Could not initialize WebGPU!" << std::endl;
    return 1;
  }
  std::cout << "WGPU instance: " << instance << std::endl;

  wgpuInstanceRelease(instance);

  glfwTerminate();

  return 0;
}
