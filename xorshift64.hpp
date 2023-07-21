#ifndef XOR_SHIFT_64_HPP_
#define XOR_SHIFT_64_HPP_

#include <cstdint>
#include <time.h>

/* 乱数エンジンクラス。 */
class XorShift64 {
 public:
  using result_type = uint_fast64_t; // random_engineとして必要なエイリアス。

  constexpr XorShift64() : a_(314159265) {}

  constexpr XorShift64(result_type seed) : a_(seed) {
    a_ = (a_ == 0) ? 314159265 : a_; // a_が0だと乱数がずっと0になるので、テキトーな値にする。
  }  

  result_type operator()() {
    result_type x = a_;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    return (a_ = x);
  }

  static constexpr result_type min() { return (result_type)0; }

  static constexpr result_type max() { return (result_type)0xFFFFFFFFFFFFFFFF; }

 private:
  result_type a_;
};

#endif // XOR_SHIFT_64_HPP_
