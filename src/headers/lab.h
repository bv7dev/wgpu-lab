#ifndef WGPU_LAB_MAIN_H
#define WGPU_LAB_MAIN_H

#include "lab_buffer.h"
#include "lab_pipeline.h"
#include "lab_shader.h"
#include "lab_surface.h"
#include "lab_webgpu.h"
#include "lab_window.h"

#include <chrono>
#include <thread>

using namespace std::chrono_literals;

namespace lab {

// Automatically called by Window and Webgpu constructors
//  - generally lab user should not need to call this
//  - returns `false` when init has been previously called, or has failed.
bool init_lab();

// Processes GLFW Events and manages open Windows (and closes them)
//  - generally should be called once in the main loop
// ```cpp
// while(lab::tick()) {
//   pipeline.render_frame(surface); // render artworks
// }
// ```
bool tick();

// Wrapper for visual simplicity around `std::this_thread::sleep_for(duration);`
// ```cpp
// lab::sleep(16ms);
// ```
template<class _Rep, class _Period>
inline void sleep(const std::chrono::duration<_Rep, _Period>& d) {
  std::this_thread::sleep_for(d);
}

} // namespace lab

#endif // WGPU_LAB_MAIN_H
