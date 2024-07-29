#include <lab>

#include <thread>

int main() {
  lab::Window window("My window", 400, 280);
  lab::Webgpu webgpu("My Instance");
  lab::Shader shader("My shader", "shaders/test1.wgsl");

  lab::Surface surface(window, webgpu);
  lab::Pipeline pipeline(shader, webgpu);

  lab::ReadableBuffer<int> buffer("My Buffer", webgpu);
  auto write_buffer = [](auto&& vmap) {
    for (auto i = 0; i < vmap.capacity(); ++i) {
      vmap.push(i + 1);
    }
  };
  buffer.to_device(1024, write_buffer);

  // Read buffer ---------------------------
  bool reading_done = true;

  for (int i = 0; i < 100; ++i) {
    webgpu.device.tick();
    lab::tick();
  }

  buffer.read_async(4, 256, [&](auto&& vmap) {
    std::cout << "vmap-size: " << vmap.size() << std::endl;
    std::cout << "vmap-size: " << vmap.size() << std::endl;
    std::cout << "\nbuffer read callback: ";
    for (auto& e : vmap) {
      if ((e & 0xF) == 0) {
        std::cout << e << " ";
      }
      lab::sleep(4ms); // simulate slow transfer
      // TODO: Investigate: seems to sleep much longer than 4ms
    }
    std::cout << std::endl;
    reading_done = true;
  });

  while (lab::tick() && (!reading_done)) {
    pipeline.render_frame(surface);
    webgpu.device.tick();
    std::cout << ".";
    lab::sleep(20ms);
  }

  std::cout << "\n\nFIN !!!\n" << std::endl;

  // TODO: Investigate: weirdly, no while-tick loop needed and
  // read_async call turns out to be sync
}
