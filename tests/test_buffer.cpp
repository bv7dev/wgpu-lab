#include <lab>

int main() {
  lab::Webgpu webgpu{"My Instance"};

  auto write_buffer = [](auto& vmap) {
    for (int i = 0; i < vmap.capacity(); ++i) {
      vmap.push(i + 1);
    }
  };
  lab::ReadableBuffer<int> buffer(webgpu, 1024, write_buffer);

  buffer.read_async(20, 4, [](auto& vmap) {
    std::cout << "read callback: ";
    for (auto& e : vmap) {
      std::cout << e;
      if ((&e) != vmap.end() - 1) {
        std::cout << ", ";
      }
    }
    std::cout << std::endl;
  });

  while (lab::tick()) {
    lab::sleep(16ms);
  }
}
