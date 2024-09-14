#include <lab>
using namespace lab;

int main() {
  Window window("press space to say hello", 640, 400);

  window.set_key_callback([&window](auto& event) {
    if (event == KeyEvent{KeyCode::space, KeyAction::release}) {
      window.set_title("world!");
    }
  });

  while (tick()) {
    lab::sleep(50ms); // saves my laptop from overheating
  }
}
