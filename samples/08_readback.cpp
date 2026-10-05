// 08: getting data back from the gpu, without a window

#include <lab>

#include <iostream>
#include <numeric>
#include <span>
#include <vector>

template<typename T>
void print(const char* what, std::span<const T> values) {
  std::cout << what << ":";
  for (const T& value : values) {
    std::cout << " " << value;
  }
  std::cout << "\n";
}

int main() {
  lab::Gpu gpu;

  // A buffer of 16 numbers that are written directly into gpu memory when it is created.
  // "Storage" is the kind of buffer a compute shader would work on.
  lab::Buffer<int> buffer(gpu, 16, wgpu::BufferUsage::Storage,
                          [](std::span<int> mapped) { std::iota(mapped.begin(), mapped.end(), 0); });

  // write() replaces elements: one at index 3, then three starting at index 8
  buffer.write(-1, 3);
  buffer.write(std::vector<int>{100, 200, 300}, 8);

  // read() copies elements back and waits until the gpu is done with them
  std::vector<int> everything = buffer.read();
  print<int>("the whole buffer", everything);

  std::vector<int> part = buffer.read(8, 3);
  print<int>("elements 8 to 10", part);

  // read_async() does not wait: the callback runs during a later poll() (or lab::tick()).
  // A program that renders at the same time would not skip a beat.
  bool done = false;
  buffer.read_async([&done](std::span<const int> elements) {
    print<int>("read without blocking", elements);
    done = true;
  });
  while (!done) {
    gpu.poll();
  }
}
