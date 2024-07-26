#ifndef WGPU_LAB_STATE_H
#define WGPU_LAB_STATE_H

#include <unordered_map>

namespace lab {

using Handle = void*;
using Objmap = std::unordered_map<Handle, Handle>;

extern struct State {
  bool glfw_init = false;
  Objmap window_map{};
} state;

} // namespace lab

#endif // WGPU_LAB_STATE_H
