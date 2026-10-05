// 01: a window and keyboard input, no GPU involved yet

#include <lab>

int main() {
  lab::Window window("press space to say hello", 640, 400);

  // an event callback: called from within lab::tick() whenever a key changes
  window.on_key([&window](const lab::KeyEvent& event) {
    if (event == lab::KeyEvent{lab::KeyCode::space, lab::KeyAction::release}) {
      window.set_title("world!");
    }
  });

  // tick() processes the events of all windows and returns false once every window is closed
  while (lab::tick()) {
    // the state of a key can also be asked for directly
    if (window.key_pressed(lab::KeyCode::escape)) {
      window.close();
    }
  }
}
