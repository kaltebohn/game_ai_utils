#ifndef OTHELLO_OBSERVATION_HPP_
#define OTHELLO_OBSERVATION_HPP_

#include <vector>

#include "othello_types.hpp"

struct OthelloObservation {
  bitboard black_board_;
  bitboard white_board_;
  int cur_turn_;
  std::vector<coord> legal_actions_;
};

#endif // OTHELLO_OBSERVATION_HPP_
