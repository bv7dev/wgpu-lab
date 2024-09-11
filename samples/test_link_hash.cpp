#include <format>
#include <iostream>
#include <string>

#pragma region // unsigned types, 2, 4 and 8 bits

// struct ui2 {
//   ui2() = default;
//   ui2(std::integral auto v) : value{uint8_t(v & 0x3u)} {}
//   operator uint8_t() const { return value; }
//   uint8_t operator++() {
//     value = ++value & 0x3u;
//     return value;
//   }
//   uint8_t operator++(int) {
//     uint8_t old = value;
//     operator++();
//     return old;
//   }
//   uint8_t value;
// };
// template<>
// struct std::formatter<ui2> : std::formatter<std::string> {
//   auto format(ui2 i, format_context& ctx) const {
//     return formatter<string>::format(std::format("{:1d} \x1b[90m{:02b}\x1b[0m", i.value, i.value), ctx);
//   }
// };
// std::ostream& operator<<(std::ostream& o, ui2 i) { return o << unsigned(i.value); }

// struct ui4 {
//   ui4() = default;
//   ui4(std::integral auto v) : value{uint8_t(v & 0xfu)} {}
//   ui4(const ui2& high, const ui2& low) : value{uint8_t(high.value << 2 | low.value)} {}
//   ui4(const ui2& low) : value{low.value} {}
//   operator uint8_t() const { return value; }
//   operator ui2() const { return ui2(value); }
//   uint8_t operator++() {
//     value = ++value & 0xfu;
//     return value;
//   }
//   uint8_t operator++(int) {
//     uint8_t old = value;
//     operator++();
//     return old;
//   }
//   ui2 high() { return ui2(value >> 2); }
//   ui2 low() { return ui2(value); }
//   uint8_t value;
// };
// template<>
// struct std::formatter<ui4> : std::formatter<std::string> {
//   auto format(ui4 i, format_context& ctx) const {
//     return formatter<string>::format(std::format("{:2d} \x1b[90m{:04b}\x1b[0m", i.value, i.value), ctx);
//   }
// };
// std::ostream& operator<<(std::ostream& o, ui4 i) { return o << unsigned(i.value); }

// struct ui8 {
//   ui8() = default;
//   ui8(std::integral auto v) : value{uint8_t(v)} {}
//   ui8(const ui4& high, const ui4& low) : value{uint8_t(high.value << 4 | low.value)} {}
//   ui8(const ui4& low) : value{low.value} {}
//   operator uint8_t() const { return value; }
//   operator ui4() const { return ui4(value); }
//   uint8_t operator++() { return ++value; }
//   uint8_t operator++(int) { return value++; }
//   ui4 high() { return ui4(value >> 4); }
//   ui4 low() { return ui4(value); }
//   uint8_t value;
// };
// template<>
// struct std::formatter<ui8> : std::formatter<std::string> {
//   auto format(ui8 i, format_context& ctx) const {
//     return formatter<string>::format(std::format("{:3d} \x1b[90m{:08b}\x1b[0m", i.value, i.value), ctx);
//   }
// };
// std::ostream& operator<<(std::ostream& o, ui8 i) { return o << unsigned(i.value); }

#pragma endregion
#pragma region // generic signed or unsigned integral type with variable bit length

template<auto V>
constexpr bool is_power_of_two = V == 0 || V == 1 || V && !(V & (V - 1));

template<std::integral T, unsigned Bits>
  requires is_power_of_two<Bits>
struct Int {
  static const unsigned BitMask = (1 << Bits) - 1, HalfBits = Bits >> 1;
  Int() = default;
  Int(const std::integral auto& v) : value{T(v & BitMask)} {}
  Int(const Int<T, HalfBits>& high, const Int<T, HalfBits>& low) : value{T((high.value << HalfBits) | low.value)} {}
  Int(const Int& low) : value{low.value} {}
  operator T() const { return value; }
  operator Int<T, (Bits >> 1)>() const { return Int<T, (Bits >> 1)>(value); }
  T operator++() {
    value = ++value & ((1 << Bits) - 1);
    return value;
  }
  T operator++(int) {
    T tmp = value;
    operator++();
    return tmp;
  }
  Int<T, (Bits >> 1)> high() { return Int<T, (Bits >> 1)>(value >> (Bits >> 1)); }
  Int<T, (Bits >> 1)> low() { return Int<T, (Bits >> 1)>(value); }
  T value;
};
template<std::integral T, unsigned Bits>
struct std::formatter<Int<T, Bits>> : std::formatter<std::string> {
  auto format(Int<T, Bits> i, format_context& ctx) const {
    auto maxlen = std::to_string((size_t(1u) << Bits) - 1).size();
    auto fmtstr = std::format("{{:{}d}} \x1b[90m{{:0{}b}}\x1b[0m", maxlen, Bits);
    return formatter<string>::format(std::vformat(fmtstr, std::make_format_args(i.value, i.value)), ctx);
  }
};
template<std::integral T, unsigned Bits>
std::ostream& operator<<(std::ostream& o, Int<T, Bits> i) {
  return o << std::format("{}", i);
}

using ui2 = Int<uint8_t, 2>;
using ui4 = Int<uint8_t, 4>;
using ui8 = Int<uint8_t, 8>;

#pragma endregion

int main() {
#pragma region // experiment with bit patterns

  for (int i = 0; i < 4; ++i) {
    for (ui4 a = 0; a < 4; ++a) {
      for (ui4 b = 0; b < 4; ++b) {
        if ((a ^ b) == i) {
          ui4 link{ui2(a), ui2(b)};
          std::cout << std::format("{:>7}   {:>1} {:>1}   {:>7}\n", link, int(a), int(b), ui4(ui2(a) + ui2(b)));
        }
      }
    }
    std::cout << std::endl;
  }

#pragma endregion
#pragma region // find 4 bit hash functions

  // using hashfunc = ui8 (*)(ui8, ui8);
  // hashfunc hashfuncs[] = {
  //     [](ui8 a, ui8 b) { return ui8(((a + b) >> 2) & 0x7u); },
  //     [](ui8 a, ui8 b) { return ui8(((a + b) >> 2) & 0x7u); },
  //     [](ui8 a, ui8 b) { return ui8(((a * b) >> 1) & 0x7u); },
  //     [](ui8 a, ui8 b) { return ui8((((a + b) >> 2) ^ (((a + b) >> 1) & 0x1u)) & 0x7u); },
  //     [](ui8 a, ui8 b) { return ui8(((a * b) >> 1) & 0x7u); },
  //     [](ui8 a, ui8 b) { return ui8((((a * b) >> 1) ^ (((a * b) >> 4) & 0x1u) ^ (((a * b) >> 7) & 0x1u)) & 0x7u); },
  //     [](ui8 a, ui8 b) { return ui8(((a * b) >> 1) & 0x7u); },
  //     [](ui8 a, ui8 b) { return ui8((((a + b) >> 1) ^ (((a + b) >> 4) & 0x1u)) & 0x7u); },
  //     [](ui8 a, ui8 b) { return ui8(((a * b) >> 1) & 0x7u); },
  // };

  // for (int i = 1; i <= 8; ++i) {
  //   std::cout << std::format("link     a  b     a^b   a@b\n");
  //   for (ui8 a = 0; a < 16; ++a) {
  //     for (ui8 b = 0; b < 16; ++b) {
  //       ui8 link = {ui4(a), ui4(b)};
  //       if ((a ^ b) == i) {
  //         std::cout << std::format("{:>3}     {:>2} {:>2}     {:>2}   {:>12}\n", int(link), int(a), int(b), a ^ b,
  //                                  hashfuncs[i - 1](a, b));
  //       }
  //     }
  //   }
  //   std::cout << "\n\n";
  // }

#pragma endregion
}
