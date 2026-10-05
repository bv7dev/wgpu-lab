#ifndef WGPU_LAB_LOG_H
#define WGPU_LAB_LOG_H

#include <functional>
#include <string_view>

namespace lab {

enum class LogLevel { debug, info, warn, error, off };

// Messages below this level are dropped
//  - the default is `info`, or what the environment variable LAB_LOG says
//    (debug, info, warn, error or off)
void set_log_level(LogLevel level);
LogLevel log_level();

// Decides where messages go, by default they are printed to stderr
// ```cpp
// lab::set_log_sink([&](lab::LogLevel level, std::string_view message) { my_console.add(message); });
// ```
void set_log_sink(std::function<void(LogLevel, std::string_view)> sink);

void log(LogLevel level, std::string_view message);

} // namespace lab

#endif // WGPU_LAB_LOG_H
