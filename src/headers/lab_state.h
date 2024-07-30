#ifndef WGPU_LAB_STATE_H
#define WGPU_LAB_STATE_H

#include <unordered_map>

// Note: user should not need to interact with state directly -> not in lab.h

namespace lab {

using Handle = void*;
using Objmap = std::unordered_map<Handle, Handle>; // GLFWwindow* -> lab::Window*
// todo: replace by more descriptive handels to get rid of many reinterpret casts

extern struct State {
  bool glfw_init = false;
  Objmap window_map{};
} state;

} // namespace lab

#endif // WGPU_LAB_STATE_H
