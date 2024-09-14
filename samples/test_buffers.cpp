#include <lab>

// It's foolish to send std::strings to a gpu buffer.
// But why not =)

int main() {
  lab::Webgpu webgpu("My Instance");

  // Init Buffer ---------------------------------------------------------------
  std::cout << "\n\nWriting Buffer...\n";
  lab::Buffer<std::string> buffer("My Buffer", webgpu);
  {
    bool writing_done = false;
    auto init_buffer = [&writing_done](lab::MappedVRAM<std::string> vmap) {
      int start_point = 6;
      vmap.resize(start_point);
      for (int i = 0; i < 100; ++i) {
        lab::sleep(10ms); // simulate slow loading, converting, etc.
        std::cout << vmap.push(std::to_string(i + 5)) << " ";
      }
      std::cout << "\nbuffer initialized\n";
      writing_done = true;
    };
    auto thread = buffer.to_device(init_buffer, 256, wgpu::BufferUsage::MapRead);
    std::cout << "Main thread waiting...";
    while (!writing_done) {
      lab::sleep(20ms);
      std::cout << ".";
    }
  }
  // this buffer is just to see if something breaks
  lab::Buffer<std::string> another("Stress Test", std::vector<std::string>{"1", "2", "3", "4+"},
                                   wgpu::BufferUsage::MapRead, webgpu);

  // Read buffer ---------------------------------------------------------------
  bool reading_done = false;
  auto read_buffer = [&reading_done](lab::MappedVRAM<const std::string> vmap) {
    for (auto& e : vmap) {
      lab::sleep(50ms); // simulate slow processing of received data
      std::cout << e << " ";
    }

    // unmap can be called when done reading or writing, otherwise
    vmap.unmap(); // it will be automatically unmapped when exiting this function

    std::cout << std::endl;
    reading_done = true;
  };

  std::cout << "\n\nReading Buffer...\n";
  {
    auto thread = buffer.from_device(2, 200, read_buffer);

    std::cout << "Main thread waiting...";
    while (!reading_done) {
      lab::sleep(10ms);
      std::cout << ".";
    }
  }

  reading_done = false;
  auto thread = another.from_device(read_buffer);
  while (!reading_done) {
  }
  std::cout << "\nFIN !!!\n" << std::endl;
}
