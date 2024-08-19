// To keep an overview over all public dependencies
// that are directly exposed to the user

// STL dependencies ------------------------------

#include <cassert>
#include <chrono>
#include <cmath>
#include <concepts>
#include <fstream>
#include <functional>
#include <iostream>
#include <ranges>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>

#ifdef LAB_USE_STL_CHRONO_LITERALS
using namespace std::chrono_literals;
#endif

// Dawn dependencies
#include <webgpu/webgpu.hpp>

// Lab objects and enums -------------------------

#include <objects/lab_buffer.h>
#include <objects/lab_pipeline.h>
#include <objects/lab_shader.h>
#include <objects/lab_surface.h>
#include <objects/lab_texture.h>
#include <objects/lab_webgpu.h>
#include <objects/lab_window.h>

#include <extra/lab_enums.h>
