#include <lab>

int main() {
  // TEST 1 - Load strings onto GPU buffers - this evil creation should not be possible
  {
    lab::Webgpu webgpu("My Instance");

    // Init Buffer ---------------------------------------------------------------
    bool writing_done = false;
    std::cout << "\n\nWriting Buffer...\n";
    lab::ReadableBuffer<std::string> buffer("My Buffer", webgpu);

    auto init_buffer = [&writing_done](lab::MappedVRAM<std::string>&& vmap) {
      int start_point = 6;
      vmap.resize(start_point);
      for (int i = 0; i < 100; ++i) {
        lab::sleep(10ms); // simulate slow loading, converting, etc.
        std::cout << vmap.push(std::to_string(i + 5)) << " ";
      }
      std::cout << "\nbuffer initialized\n";
      writing_done = true;
    };
    buffer.to_device(256, init_buffer);

    std::cout << "Main thread waiting...";
    while (!writing_done) {
      lab::sleep(20ms);
      std::cout << ".";
    }

    // this buffer does nothing, it's just to see if something breaks
    lab::ReadableBuffer<std::string> another("Stress Test",
                                             std::vector<std::string>{"1", "2", "3", "4+"}, webgpu);

    // Read buffer ---------------------------------------------------------------
    bool reading_done = false;
    std::cout << "\n\nReading Buffer...\n";
    auto read_buffer = [&reading_done](lab::ConstMappedVRAM<std::string>&& vmap) {
      for (auto e : vmap) {
        lab::sleep(50ms); // simulate slow processing of received data
        std::cout << e << " ";
      }

      // unmap CAN be called when done reading or writing, otherwise
      vmap.unmap(); // it will be automatically unmapped on return
      // I found a way with mutable to make unmap const so that it's callable here

      std::cout << std::endl;
      reading_done = true;
    };
    buffer.from_device(2, 200, read_buffer);

    std::cout << "Main thread waiting...";
    while (!reading_done) {
      lab::sleep(10ms);
      std::cout << ".";
    }
    std::cout << "\nFIN !!!\n" << std::endl;

    reading_done = false;
    another.from_device(read_buffer);
    while (!reading_done) {
    }
    // end stress test
  }

  // Test 2 ...
  {
    // ...
  }
}
