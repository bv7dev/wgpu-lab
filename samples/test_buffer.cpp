#include <lab>

int main() {
  lab::Webgpu webgpu("My Instance");

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
      if ((e & 0xF) == 0) std::cout << e << " ";
      lab::sleep(50ms);
    }
    std::cout << std::endl;
    reading_done = true;
  });

  std::cout << "waiting";
  while (!reading_done) {
    lab::sleep(200ms);
    std::cout << ".";
  }
  std::cout << "\nFIN !!!\n" << std::endl;
}
