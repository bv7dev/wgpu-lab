#include <format>
#include <iostream>
#include <string>

#pragma region // 4 and 8 bit unsigned types
struct ui4 {
  ui4() = default;
  ui4(std::integral auto v) : value{uint8_t(v & 0xfu)} {}
  operator uint8_t() const { return value; }
  uint8_t operator++() {
    value = ++value & 0xfu;
    return value;
  }
  uint8_t operator++(int) {
    uint8_t old = value;
    operator++();
    return old;
  }
  uint8_t value;
};
template<>
struct std::formatter<ui4> : std::formatter<std::string> {
  auto format(ui4 i, format_context& ctx) const {
    return formatter<string>::format(std::format("{:2d} \x1b[90m{:04b}\x1b[0m", i.value, i.value), ctx);
  }
};
std::ostream& operator<<(std::ostream& o, ui4 i) { return o << unsigned(i.value); }

struct ui8 {
  ui8() = default;
  ui8(const ui4& high, const ui4& low) : value{uint8_t(high.value << 4 | low.value)} {}
  ui8(const ui4& low) : value{low.value} {}
  ui8(std::integral auto v) : value{uint8_t(v)} {}
  operator uint8_t() const { return value; }
  operator ui4() const { return ui4(value); }
  uint8_t operator++() { return ++value; }
  uint8_t operator++(int) { return value++; }
  ui4 high() { return ui4(value >> 4); }
  ui4 low() { return ui4(value); }
  uint8_t value;
};
template<>
struct std::formatter<ui8> : std::formatter<std::string> {
  auto format(ui8 i, format_context& ctx) const {
    return formatter<string>::format(std::format("{:3d} \x1b[90m{:08b}\x1b[0m", i.value, i.value), ctx);
  }
};
std::ostream& operator<<(std::ostream& o, ui8 i) { return o << unsigned(i.value); }
#pragma endregion

int main() {
  using hashfunc = ui8 (*)(ui8, ui8);
  hashfunc hashfuncs[] = {
      [](ui8 a, ui8 b) { return ui8(((a + b) >> 2) & 0x7u); },
      [](ui8 a, ui8 b) { return ui8(((a + b) >> 2) & 0x7u); },
      [](ui8 a, ui8 b) { return ui8(((a * b) >> 1) & 0x7u); },
      [](ui8 a, ui8 b) { return ui8((((a + b) >> 2) ^ (((a + b) >> 1) & 0x1u)) & 0x7u); },
      [](ui8 a, ui8 b) { return ui8(((a * b) >> 1) & 0x7u); },
      [](ui8 a, ui8 b) { return ui8((((a * b) >> 1) ^ (((a * b) >> 4) & 0x1u) ^ (((a * b) >> 7) & 0x1u)) & 0x7u); },
      [](ui8 a, ui8 b) { return ui8(((a * b) >> 1) & 0x7u); },
      [](ui8 a, ui8 b) { return ui8((((a + b) >> 1) ^ (((a + b) >> 4) & 0x1u)) & 0x7u); },
  };

  for (int i = 1; i <= 8; ++i) {
    std::cout << std::format("link     a  b     a^b   a@b\n");
    for (ui8 a = 0; a < 16; ++a) {
      for (ui8 b = 0; b < 16; ++b) {
        ui8 link = {ui4(a), ui4(b)};
        if ((a ^ b) == i) {
          std::cout << std::format("{:>3}     {:>2} {:>2}     {:>2}   {:>12}\n", int(link), int(a), int(b), a ^ b,
                                   hashfuncs[i - 1](a, b));
        }
      }
    }
    std::cout << "\n\n";
  }
}
