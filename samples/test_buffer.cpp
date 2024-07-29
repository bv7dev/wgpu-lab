#include <lab>

#include <thread>

int main() {
  lab::Window window("My window", 400, 280);
  lab::Webgpu webgpu("My Instance");
  lab::Shader shader("My shader", "shaders/test1.wgsl");

  lab::Surface surface(window, webgpu);
  lab::Pipeline pipeline(shader, webgpu);

  // Write Buffer ---------------------------------------
  lab::ReadableBuffer<int> buffer("My Buffer", webgpu);
  auto write_buffer = [](auto&& vmap) {
    for (auto i = 0; i < vmap.capacity(); ++i) {
      vmap.push(i + 1);
    }
  };
  buffer.to_device(1024, write_buffer);

  // Read buffer -------------------------------------------
  bool reading_done = false;

  buffer.read_async(2, 256, [&](auto&& vmap) {
    for (auto& e : vmap) {
      if ((e & 0xF) == 0) {
        std::cout << e << " ";
      }
      // Investigate: seems to sleep much longer than 1ms ----------------
      lab::sleep(1ms); // simulate slow data processing
      // Turns out that sleep is not really good for precise time control.
      // I guess for testing and experimenting it's fine though.
    }
    std::cout << std::endl;
    reading_done = true;

    for (int i = 0; i < 100; i++) {
      std::cout << "+ ";
      lab::sleep(20ms);
    }
  });

  while (lab::tick()) {
    pipeline.render_frame(surface);
    webgpu.device.tick();
    std::cout << ".";
    lab::sleep(20ms);
  }

  std::cout << "\n\nFIN !!!\n" << std::endl;
}
