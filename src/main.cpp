#include <iostream>

#include <GLFW/glfw3.h>
#include <webgpu/webgpu.h>

#include "adapter.h"

int main(int argc, char** argv) {
  std::cout << "Hi!" << std::endl;
  for (int i = 0; i < argc; ++i)
    std::cout << argv[i] << ' ';
  std::cout << std::endl;

  if (!glfwInit()) return 1;
  GLFWwindow* window = glfwCreateWindow(640, 400, "Hi!", NULL, NULL);
  if (!window) {
    std::cerr << "Error: Could not initialize GLFW!" << std::endl;
    glfwTerminate();
    return 1;
  }

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
  }
  glfwTerminate();

  WGPUInstanceDescriptor desc = {};
  WGPUInstance instance = wgpuCreateInstance(&desc);
  if (!instance) {
    std::cerr << "Error: Could not initialize WebGPU!" << std::endl;
    return 1;
  }
  std::cout << "WGPU instance: " << instance << std::endl;
  std::cout << "Requesting adapter..." << std::endl;

  WGPURequestAdapterOptions adapterOpts = {};
  WGPUAdapter adapter = requestAdapterSync(instance, &adapterOpts);

  std::cout << "Got adapter: " << adapter << std::endl;

  wgpuInstanceRelease(instance);

  return 0;
}
