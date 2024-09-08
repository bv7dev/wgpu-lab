#include <format>
#include <iostream>

struct ui4 {
  ui4(std::integral auto v) : value{uint8_t(v & 0xfu)} {}
  operator uint8_t() const { return value; }
  uint8_t value;
};
template<>
struct std::formatter<ui4> : std::formatter<std::string> {
  auto format(ui4 i, format_context& ctx) const { return formatter<string>::format(std::format("{}", i.value), ctx); }
};
std::ostream& operator<<(std::ostream& o, ui4 i) { return o << i.value; }

struct ui8 {
  ui8(const ui4& high, const ui4& low) : value{uint8_t(high.value << 4 | low.value)} {}
  ui8(std::integral auto v) : value{uint8_t(v)} {}
  operator uint8_t() const { return value; }
  ui4 high() { return ui4(value >> 4); }
  ui4 low() { return ui4(value); }
  uint8_t value;
};
template<>
struct std::formatter<ui8> : std::formatter<std::string> {
  auto format(ui8 i, format_context& ctx) const { return formatter<string>::format(std::format("{}", i.value), ctx); }
};
std::ostream& operator<<(std::ostream& o, ui8 i) { return o << i.value; }

struct Link {
  ui8 key;
};

int main() {
  using namespace std;

  ui4 a = 5;
  ui4 b = 35;
  ui8 c{b, a};
  ui8 d{a, b};

  uint8_t x = c + 3;
  uint8_t y = ui8(a, b);
  uint8_t z = a + b;
  ui4 w = a + d;

  std::cout << int(w) << int(c) << int(a) << std::endl;

  cout << format("{}, {}, {}, {}\n{}, {}, {}, {}\n", a, b.value, c.value, d.value, x, y, z, w);
}
