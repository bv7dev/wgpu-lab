#ifndef WGPU_LAB_MAIN_H
#define WGPU_LAB_MAIN_H

#include "lab_surface.h"
#include "lab_webgpu.h"
#include "lab_window.h"

#include <chrono>
#include <thread>

using namespace std::chrono_literals;

namespace lab {

using Handle = void*;

template <class _Rep, class _Period> inline void sleep(const std::chrono::duration<_Rep, _Period>& d) { std::this_thread::sleep_for(d); }

bool tick();
void link(Window&, Webgpu&);

void swt(Window&, Webgpu&);

} // namespace lab

#endif // WGPU_LAB_MAIN_H
