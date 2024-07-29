#include <lab>

#include <thread>

int main() {
  lab::Window window("My window", 400, 280);
  lab::Webgpu webgpu("My Instance");
  lab::Shader shader("My shader", "shaders/test1.wgsl");

  lab::Surface surface(window, webgpu);
  lab::Pipeline pipeline(shader, webgpu);

  lab::ReadableBuffer<int> buffer("My Buffer", webgpu);
  auto write_buffer = [](auto& vmap) {
    for (auto i = 0; i < vmap.capacity(); ++i) {
      vmap.push(i + 1);
    }
  };
  buffer.to_device(1024, write_buffer);

  buffer.wgpu_buffer.unmap();

  // Read buffer ---------------------------
  bool reading_done = false;
  bool thread_calls_read_async = false;
  bool thread_call_done = false;

  std::thread t1([&]() { // try other thread
    lab::sleep(2s);
    thread_calls_read_async = true;
    std::cout << "\nt1 calls read_async now..." << std::endl;
    auto cb = buffer.read_async(2, 356, [&reading_done](auto& vmap) {
      std::cout << "buffer read callback: ";
      for (auto& e : vmap) {
        std::cout << e << " ";
        lab::sleep(2ms); // simulate slow transfer
      }
      std::cout << std::endl;
      reading_done = true;
    });
    thread_call_done = true;
  });

  // Threading tests:
  // it is possible to launch read_async on another thread, but unmapping needs
  // to happen on the main thread and can cause exceptions done too soon.

  int tick = 0;

  while (lab::tick() && (!reading_done)) {
    pipeline.render_frame(surface);
    webgpu.device.tick();
    std::cout << ".";
    lab::sleep(20ms);
    ++tick;

    if (thread_calls_read_async) {
      std::cout << "\n_#!(#!_)**$!\n" << std::endl;
      thread_calls_read_async = false;
    }

    if (thread_call_done) {
      t1.join();
      thread_call_done = false;
    }
  }

  lab::sleep(2s);
  buffer.wgpu_buffer.unmap();

  std::cout << "\n\nFIN !!!\n" << std::endl;

  // TODO: Investigate: weirdly, no while-tick loop needed and
  // read_async call turns out to be sync
}
