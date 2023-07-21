#ifndef OTHELLO_TYPES_HPP_
#define OTHELLO_TYPES_HPP_

#include <stdint.h>
#include <utility>

using bitboard = uint64_t;
using coord = std::pair<int, int>;  // A4は{0, 3} で表現。

#endif // OTHELLO_TYPES_HPP_
