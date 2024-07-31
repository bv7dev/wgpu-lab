#ifndef WGPU_LAB_STATE_H
#define WGPU_LAB_STATE_H

#include "lab_objects.h"

#include <unordered_map>

// Note: user should not interact with state directly
//       so it's not included in lab.h

namespace lab {

using WindowMap = std::unordered_map<GlfwWindowHandle, WindowHandle>;

extern struct State {
  WindowMap window_map{};
  bool glfw_init = false;
} state;

} // namespace lab

#endif // WGPU_LAB_STATE_H
