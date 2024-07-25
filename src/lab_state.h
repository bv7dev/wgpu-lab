#ifndef WGPU_LAB_STATE_H
#define WGPU_LAB_STATE_H

#include <unordered_map>

namespace lab {

using Handle = void*;

struct State {
  std::unordered_map<Handle, Handle> window_map;
  bool init = false;
};

} // namespace lab

#endif // WGPU_LAB_STATE_H
