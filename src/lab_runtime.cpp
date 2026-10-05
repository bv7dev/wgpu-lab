#include "lab_detail.h"

#include <lab>

#include <GLFW/glfw3.h>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

namespace lab {

// logging ---------------------------------------------------------------------------------------

namespace {

LogLevel initial_log_level() {
  const char* value = std::getenv("LAB_LOG");
  const std::string name = value ? value : "";
  if (name == "debug") return LogLevel::debug;
  if (name == "warn") return LogLevel::warn;
  if (name == "error") return LogLevel::error;
  if (name == "off") return LogLevel::off;
  return LogLevel::info;
}

LogLevel current_log_level = initial_log_level();

void print_to_stderr(LogLevel level, std::string_view message) {
  static constexpr const char* names[] = {"debug", "info", "warn", "error"};
  std::cerr << "lab [" << names[static_cast<int>(level)] << "] " << message << std::endl;
}

std::function<void(LogLevel, std::string_view)> log_sink = print_to_stderr;

} // namespace

void set_log_level(LogLevel level) { current_log_level = level; }
LogLevel log_level() { return current_log_level; }

void set_log_sink(std::function<void(LogLevel, std::string_view)> sink) {
  log_sink = sink ? std::move(sink) : print_to_stderr;
}

void log(LogLevel level, std::string_view message) {
  if (level >= current_log_level && level != LogLevel::off) {
    log_sink(level, message);
  }
}

// errors and labels ------------------------------------------------------------------------------

namespace detail {

void fail(std::string_view object, std::string_view message) {
  std::string what = std::format("{}: {}", object, message);
  lab::log(LogLevel::error, what);
  throw Error(what);
}

} // namespace detail

Label::Label(std::source_location where)
    : text{std::format("{}:{}", std::filesystem::path(where.file_name()).filename().string(), where.line())},
      from_location{true} {}

Label&& Label::as(std::string_view kind) && {
  if (from_location) {
    text = std::format("{} {}", kind, text);
    from_location = false;
  }
  return std::move(*this);
}

// runtime ---------------------------------------------------------------------------------------

namespace detail {

Runtime& Runtime::get() {
  static Runtime runtime;
  return runtime;
}

Runtime::Runtime() {
  if (const char* value = std::getenv("LAB_EXIT_AFTER_FRAMES")) {
    exit_after_frames = std::atol(value);
  }
}

Runtime::~Runtime() {
  if (glfw_ready) {
    glfwTerminate(); // destroys the windows that are left, too
  }
}

void Runtime::init_glfw() {
  if (glfw_ready) {
    return;
  }
  glfwSetErrorCallback(
      [](int code, const char* description) { detail::log(LogLevel::warn, "GLFW error {}: {}", code, description); });
  if (!glfwInit()) {
    fail("lab", "GLFW could not be initialized, is there a display?");
  }
  glfw_ready = true;
  log(LogLevel::debug, "GLFW {} initialized", glfwGetVersionString());
}

void Runtime::retire_window(GLFWwindow* window) {
  retired_windows.push_back(window);
  destroy_retired_windows();
}

void Runtime::destroy_retired_windows() {
  if (!gpus.empty()) {
    return; // a device might still own a swapchain of one of these windows
  }
  for (GLFWwindow* window : retired_windows) {
    glfwDestroyWindow(window);
  }
  retired_windows.clear();
}

double elapsed_seconds() {
  return std::chrono::duration<double>(Runtime::Clock::now() - Runtime::get().timer_start).count();
}

} // namespace detail

// main loop ---------------------------------------------------------------------------------------

bool tick() {
  detail::Runtime& runtime = detail::Runtime::get();

  for (detail::WindowState* window : runtime.windows) {
    window->begin_tick();
  }

  if (runtime.glfw_ready) {
    if (runtime.presented || runtime.windows.empty()) {
      glfwPollEvents();
    } else {
      // Nothing was drawn since the last tick: no vsync is pacing the loop (windows
      // are minimized, or the program does not render). Wait a little for events
      // instead of spinning at full speed.
      glfwWaitEventsTimeout(0.01);
    }
  }
  runtime.presented = false;

  const bool exit_requested = runtime.exit_after_frames > 0 && ++runtime.tick_count > runtime.exit_after_frames;
  if (exit_requested) {
    runtime.tick_count = 0;
  }

  // closing a window removes it from the list, so work on a copy
  for (detail::WindowState* window : std::vector(runtime.windows)) {
    if (exit_requested || glfwWindowShouldClose(window->handle)) {
      window->close();
    }
  }

  for (detail::GpuState* gpu : std::vector(runtime.gpus)) {
    gpu->poll();
  }

  const auto now = detail::Runtime::Clock::now();
  runtime.delta_seconds = std::chrono::duration<float>(now - runtime.last_tick).count();
  runtime.last_tick = now;

  return !runtime.windows.empty();
}

float delta_seconds() { return detail::Runtime::get().delta_seconds; }

void restart_timer() { detail::Runtime::get().timer_start = detail::Runtime::Clock::now(); }

} // namespace lab
