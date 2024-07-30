#include <lab>

using namespace std;
using namespace lab;

int main() {
  Webgpu webgpu("My Instance");

  // Multi-threaded reading and writing onto mapped GPU memory  ----------------
  bool writing_done = false;
  cout << "\n\nWriting Buffer...\n";
  ReadableBuffer<int> buffer("My Buffer", webgpu);

  auto init_buffer = [&writing_done](MappedVRAM<int>&& vmap) {
    int start_point = 6;
    vmap.resize(start_point);
    for (int i = 0; i < 100; ++i) {
      cout << vmap.push(i + 5) << " "; // stack some numbers on the buffer
    }
    cout << "\nbuffer initialized" << endl;
    writing_done = true;
  };
  buffer.to_device(256, init_buffer);

  cout << "Main thread waiting...";
  while (!writing_done)
    cout << '.';

  // Read the buffer back to CPU -----------------------------------------------
  bool reading_done = false;
  cout << "\n\nReading Buffer...\n";
  auto read_buffer = [&reading_done](ConstMappedVRAM<int>&& vmap) {
    for (auto e : vmap) {
      cout << e << " "; // print some numbers received from gpu
    }
    cout << "\nbuffer read" << endl;
    reading_done = true;
  };
  buffer.from_device(2, 200, read_buffer);

  cout << "Main thread waiting...";
  while (!reading_done)
    cout << ".";

  cout << "\nFIN !!!\n" << endl;
}
