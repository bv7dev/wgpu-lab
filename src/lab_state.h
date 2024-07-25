#ifndef WGPU_LAB_STATE_H
#define WGPU_LAB_STATE_H

#include <unordered_map>

namespace lab {

using Handle = void*;
using Objmap = std::unordered_map<Handle, Handle>;

struct State {
  Objmap window_map;
  bool init = false;
};

extern State state;

} // namespace lab

#endif // WGPU_LAB_STATE_H
