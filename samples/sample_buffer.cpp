#include <lab>

using namespace std;
using namespace lab;

int main() {
  Webgpu webgpu("My Instance");

  // Multi-threaded reading and writing onto mapped GPU memory  ----------------
  cout << "\n\nWriting Buffer...\n";
  Buffer<int> buffer("My Buffer", webgpu);
  {
    bool writing_done = false;
    auto init_buffer = [&writing_done](MappedVRAM<int> vmap) {
      int start_point = 6;
      vmap.resize(start_point);
      for (int i = 0; i < 10; ++i) {
        cout << vmap.push(i + 5) << " "; // stack numbers on the buffer
        sleep(100ms);
      }
      cout << "\nbuffer initialized" << endl;
      writing_done = true;
    };
    auto thread = buffer.to_device(init_buffer, 256, wgpu::BufferUsage::MapRead);

    cout << "Main thread waiting...";
    while (!writing_done) {
      cout << '.';
      sleep(50ms);
    }
  }

  // Read the buffer back to CPU -----------------------------------------------
  {
    bool reading_done = false;
    cout << "\n\nReading Buffer...\n";
    auto read_buffer = [&reading_done](MappedVRAM<const int> vmap) {
      for (auto e : vmap) {
        cout << e << " "; // print numbers received from gpu
        sleep(100ms);
      }
      cout << "\nbuffer read" << endl;
      vmap.unmap();
      reading_done = true;
    };
    auto thread = buffer.from_device(2, 20, read_buffer);

    cout << "Main thread waiting...";
    while (!reading_done) {
      cout << ".";
      sleep(200ms);
    }
  }

  cout << "\nFIN !!!\n" << endl;
}
