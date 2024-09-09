#include <format>
#include <iostream>
#include <string>

#pragma region // 4 and 8 bit unsigned types
struct ui4 {
  ui4() = default;
  ui4(std::integral auto v) : value{uint8_t(v & 0xfu)} {}
  operator uint8_t() const { return value; }
  uint8_t operator++() { return ++value; }
  uint8_t operator++(int) { return value++; }
  uint8_t value;
};
template<>
struct std::formatter<ui4> : std::formatter<std::string> {
  auto format(ui4 i, format_context& ctx) const { return formatter<string>::format(std::format("{}", i.value), ctx); }
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
  auto format(ui8 i, format_context& ctx) const { return formatter<string>::format(std::format("{}", i.value), ctx); }
};
std::ostream& operator<<(std::ostream& o, ui8 i) { return o << unsigned(i.value); }
#pragma endregion

struct Link {
  ui8 key;
};

int main() {
  for (ui8 a = 0; a < 64; ++a) {
    if (a.high() > 0) {
      std::cout << a.high() << '.';
    }
    std::cout << ui4(a) << "   ";
  }
}
