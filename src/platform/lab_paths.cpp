#include "platform/lab_platform.h"

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace lab::platform {

std::filesystem::path executable_path() {
#if defined(_WIN32)
  wchar_t buffer[MAX_PATH];
  const DWORD length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
  if (length > 0 && length < MAX_PATH) {
    return std::filesystem::path(buffer);
  }
#elif defined(__linux__)
  std::error_code error;
  const std::filesystem::path executable = std::filesystem::read_symlink("/proc/self/exe", error);
  if (!error) {
    return executable;
  }
#endif
  return {};
}

std::filesystem::path executable_directory() {
  const std::filesystem::path executable = executable_path();
  if (!executable.empty()) {
    return executable.parent_path();
  }
  std::error_code ignored;
  return std::filesystem::current_path(ignored);
}

} // namespace lab::platform
