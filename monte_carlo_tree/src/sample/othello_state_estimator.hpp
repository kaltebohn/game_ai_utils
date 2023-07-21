#ifndef OTHELLO_STATE_ESTIMATOR_
#define OTHELLO_STATE_ESTIMATOR_

#include "othello_observation.hpp"
#include "othello_state.hpp"

class OthelloStateEstimator {
 public:
  OthelloState estimate(const OthelloObservation& observation) {
    return OthelloState(
      observation.black_board_,
      observation.white_board_,
      observation.cur_turn_
    );
  }
};

#endif // OTHELLO_STATE_ESTIMATOR_
