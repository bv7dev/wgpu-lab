#include "lab_pipeline.h"

#include <iostream>

namespace lab {

Pipeline::Pipeline(const std::string& lbl) : label{lbl} {
  std::cout << "Info: WGPU: Pipeline: " << label << std::endl;
  // todo
}

Pipeline::~Pipeline() {
  // todo
}

} // namespace lab
