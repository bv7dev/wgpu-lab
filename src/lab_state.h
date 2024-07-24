#ifndef LAB_STATE_H
#define LAB_STATE_H

#include <unordered_map>

namespace lab {

using Handle = void*;

struct State {
  std::unordered_map<Handle, Handle> window_map;
  bool init = false;
};

} // namespace lab

#endif // LAB_STATE_H
