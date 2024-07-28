#include <lab>

int main() {
  lab::Webgpu webgpu("My Instance");

  lab::ReadableBuffer<int> buffer("My Buffer", webgpu);
  auto write_buffer = [](auto& vmap) {
    for (auto i = 0; i < vmap.capacity(); ++i) {
      vmap.push(i + 1);
    }
  };
  buffer.to_device(1024, write_buffer);

  // Read buffer ---------------------------
  bool reading_done = false;

  buffer.read_async(2, 356, [&reading_done](auto& vmap) {
    std::cout << "buffer read callback: ";
    for (auto& e : vmap) {
      std::cout << e << " ";
      lab::sleep(20ms); // simulate slow transfer
    }
    std::cout << std::endl;
    reading_done = true;
  });

  // TODO: Investigate: weirdly, no while-tick loop needed and read_async
  // call turns out to be sync
}
