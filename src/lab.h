#ifndef WGPU_LAB_MAIN_H
#define WGPU_LAB_MAIN_H

#include "lab_state.h"
#include "lab_webgpu.h"
#include "lab_window.h"

namespace lab {

bool tick();
void link(Window& wnd, Webgpu& wgpu);

void swt(Window& wnd, Webgpu& wgpu);

} // namespace lab

#endif // WGPU_LAB_MAIN_H
