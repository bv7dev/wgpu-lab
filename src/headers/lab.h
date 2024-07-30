#ifndef WGPU_LAB_MAIN_H
#define WGPU_LAB_MAIN_H

// Main header to import all public lab- objects and -enum-classes

// for now, it's convenient to make all stl dependencies public
#include "lab_stl_deps.h" // todo: remove

#include "lab_buffer.h"
#include "lab_enums.h"
#include "lab_pipeline.h"
#include "lab_shader.h"
#include "lab_surface.h"
#include "lab_webgpu.h"
#include "lab_window.h"

namespace lab {

// -----------------------------------------------------------------------------
// Functions with side-effects, operating on the global application state

// Automatically called by Window and Webgpu constructors
//  - generally lab user should not need to call this
//  - returns `false` when init has been previously called, or has failed.
bool init_lab();

// Processes GLFW Events, handles open Windows and closes them
//  - generally should be called once in any app main loop
//  - returns `true` as long as at least one Window is open
// ```cpp
// while(lab::tick()) {
//   pipeline.render_frame(surface); // render artworks
// }
// ```
bool tick();

// -----------------------------------------------------------------------------
// Utils independent from all other lab-objects

// Wrapper for visual simplicity around `std::this_thread::sleep_for(duration);`
// - is very inaccurate and should only be used for playing around in different threads for example
// - as far as I understand, it lowers a threads priority in a queue and can lead to longer than
//   expected delays and probably differs widely on different hardware and OS
// ```cpp
// lab::sleep(100ms);
// ```
template<class _Rep, class _Period>
inline void sleep(const std::chrono::duration<_Rep, _Period>& d) {
  std::this_thread::sleep_for(d);
}

} // namespace lab

#endif // WGPU_LAB_MAIN_H
