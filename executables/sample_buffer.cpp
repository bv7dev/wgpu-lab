#include <lab>

int main() {
  lab::Webgpu webgpu("My Instance");

  // Init Buffer ---------------------------------------------------------------
  bool writing_done = false;
  std::cout << "\n\nWriting Buffer...\n";
  lab::ReadableBuffer<int> buffer("My Buffer", webgpu);

  auto init_buffer = [&](lab::MappedVRAM<int>&& vmap) {
    for (int i = 0; i < vmap.capacity(); ++i) {
      lab::sleep(10ms); // simulate slow loading, converting, etc.
      std::cout << vmap.push(i + 1) << " ";
    }
    std::cout << "init done\n";
    writing_done = true;
  };
  buffer.to_device(256, init_buffer);

  std::cout << "Main thread waiting... ";
  while (!writing_done) {
    lab::sleep(20ms);
    std::cout << ".";
  }

  // this buffer does nothing, it's just to see if something breaks
  lab::ReadableBuffer<int> another("Stress Test", std::vector<int>{1, 2, 3, 4}, webgpu);

  // Read buffer ---------------------------------------------------------------
  bool reading_done = false;
  std::cout << "\n\nReading Buffer...\n";
  auto read_buffer = [&](lab::ConstMappedVRAM<int>&& vmap) {
    for (int e : vmap) {
      lab::sleep(50ms); // simulate slow processing of received data
      std::cout << e << " ";
    }
    std::cout << std::endl;
    reading_done = true;
  };
  buffer.from_device(2, 64, read_buffer);

  std::cout << "Main thread waiting... ";
  while (!reading_done) {
    lab::sleep(10ms);
    std::cout << ".";
  }
  std::cout << "\nFIN !!!\n" << std::endl;

  reading_done = false;
  another.from_device(0, 4, read_buffer);
  while (!reading_done) {
  }
  // end stress test
}
