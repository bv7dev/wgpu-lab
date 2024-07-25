#ifndef WGPU_LAB_PIPELINE_H
#define WGPU_LAB_PIPELINE_H

#include <string>

namespace lab {

struct Pipeline {
  std::string label;
  Pipeline(const std::string& label);
  ~Pipeline();
};

} // namespace lab

#endif // WGPU_LAB_PIPELINE_H
