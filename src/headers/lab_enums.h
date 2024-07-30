#ifndef WGPU_LAB_ENUMS_H
#define WGPU_LAB_ENUMS_H

// Copied and re-formatted from GLFW/glfw3.h
//
// todo: maybe: controller support
// todo: bi-directional map to lookup all enum fields by strings and vice versa
// todo: maybe: generator script to produce this header
//       with strings and potentially new enums
// todo: overload cout << to be able to print enums

namespace lab {

enum class InputAction {
  release = 0,
  press = 1,
  repeat = 2,
};

enum class KeyCode {
  // printable keys

  space = 32,
  apostrophe = 39,
  comma = 44,
  minus = 45,
  period = 46,
  slash = 47,
  n0 = 48,
  n1 = 49,
  n2 = 50,
  n3 = 51,
  n4 = 52,
  n5 = 53,
  n6 = 54,
  n7 = 55,
  n8 = 56,
  n9 = 57,
  semicolon = 59,
  equal = 61,
  A = 65,
  B = 66,
  C = 67,
  D = 68,
  E = 69,
  F = 70,
  G = 71,
  H = 72,
  I = 73,
  J = 74,
  K = 75,
  L = 76,
  M = 77,
  N = 78,
  O = 79,
  P = 80,
  Q = 81,
  R = 82,
  S = 83,
  T = 84,
  U = 85,
  V = 86,
  W = 87,
  X = 88,
  Y = 89,
  Z = 90,
  left_bracket = 91,
  backslash = 92,
  right_bracket = 93,
  grave_accent = 96,

  // world keys o.O

  world_1 = 161,
  world_2 = 162,

  // function keys

  escape = 256,
  enter = 257,
  tab = 258,
  backspace = 259,
  insert = 260,
  del = 261,
  right = 262,
  left = 263,
  down = 264,
  up = 265,
  page_up = 266,
  page_down = 267,
  home = 268,
  end = 269,
  caps_lock = 280,
  scroll_lock = 281,
  num_lock = 282,
  print_screen = 283,
  pause = 284,
  F1 = 290,
  F2 = 291,
  F3 = 292,
  F4 = 293,
  F5 = 294,
  F6 = 295,
  F7 = 296,
  F8 = 297,
  F9 = 298,
  F10 = 299,
  F11 = 300,
  F12 = 301,
  F13 = 302,
  F14 = 303,
  F15 = 304,
  F16 = 305,
  F17 = 306,
  F18 = 307,
  F19 = 308,
  F20 = 309,
  F21 = 310,
  F22 = 311,
  F23 = 312,
  F24 = 313,
  F25 = 314,
  pad_0 = 320,
  pad_1 = 321,
  pad_2 = 322,
  pad_3 = 323,
  pad_4 = 324,
  pad_5 = 325,
  pad_6 = 326,
  pad_7 = 327,
  pad_8 = 328,
  pad_9 = 329,
  pad_decimal = 330,
  pad_divide = 331,
  pad_multiply = 332,
  pad_subtract = 333,
  pad_add = 334,
  pad_enter = 335,
  pad_equal = 336,
  left_shift = 340,
  left_control = 341,
  left_alt = 342,
  left_super = 343,
  right_shift = 344,
  right_control = 345,
  right_alt = 346,
  right_super = 347,
  menu = 348,

  // unknown key code

  unknown = -1,
};

enum ModKey {
  none = 0x0000,
  shift = 0x0001,
  control = 0x0002,
  alt = 0x0004,
  super = 0x0008,
  caps_lock = 0x0010,
  num_lock = 0x0020,
};

enum class MouseButton {
  left = 0,
  right = 1,
  middle = 2,
  b4 = 3,
  b5 = 4,
  b6 = 5,
  b7 = 6,
  b8 = 7,
};

} // namespace lab

#endif // WGPU_LAB_ENUMS_H
