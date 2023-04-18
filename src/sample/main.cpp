#include <random>
#include <string.h>
#include <iostream>

#include "../primitive_monte_carlo_root.hpp"
#include "../monte_carlo_tree_node.hpp"
#include "othello_observation.hpp"
#include "othello_state.hpp"
#include "othello_state_estimator.hpp"

coord getPlayerInput(const OthelloState& state) {
  std::cout << "石を置く場所を指定してください。" << std::endl;
  std::cout << "着手を入力してください。" << std::endl;
  std::cout << ">> ";
  std::string input{};
  std::cin >> input;
  coord action{OthelloState::str2Coord(input)};

  while (action == std::make_pair<int, int>(-1, -1) || !state.isLegal(action)) {
    std::cout << "合法手ではありません。もう一度入力してください。" << std::endl;
    std::cin.clear();
    std::cin.ignore(256, '\n');
    std::cout << ">> ";
    std::cin >> input;
    action = OthelloState::str2Coord(input);
  }

  return action;
}

coord getPMCInput(const OthelloState& state) {
  std::random_device seed_gen; // 乱数のシード生成器。
  OthelloStateEstimator estimator{}; // 状態推定器。オセロは完全情報ゲームなので、形だけ。

  PrimitiveMonteCarloRoot<OthelloState, OthelloObservation, OthelloStateEstimator, coord, 2> node =
      PrimitiveMonteCarloRoot<OthelloState, OthelloObservation, OthelloStateEstimator, coord, 2>
      (state.getObservation(), estimator, state.getCurrentPlayerNum());
  return node.search();
}

coord getMCTSInput(const OthelloState& state) {
  /* 乱数のシード生成器。 */
  std::random_device seed_gen;

  MonteCarloTreeNode<OthelloState, coord, 2> node =
      MonteCarloTreeNode<OthelloState, coord, 2>
      (state, state.getCurrentPlayerNum(), {-1, -1}, seed_gen());
  return node.search();
}

void pvp() {
  OthelloState state{};
  while (!state.isFinished()) {
    if (state.getCurrentPlayerNum() == OthelloState::kBlackTurn) {
      std::cout << "黒の手番" << std::endl;
    } else {
      std::cout << "白の手番" << std::endl;
    }
    std::cout << std::endl;

    state.print();
    std::cout << std::endl;
    state = state.next(getPlayerInput(state));
  }

  if (state.getScore(OthelloState::kBlackTurn) == 1) {
    std::cout << "黒の勝ち" << std::endl;
  } else if (state.getScore(OthelloState::kWhiteTurn) == 1) {
    std::cout << "白の勝ち" << std::endl;
  } else {
    std::cout << "引き分け" << std::endl;
  }
}

void monte_carlo() {
  /* ゲームの状態。 */
  OthelloState state{};

  int player_color;
  int tmp;
  std::cout << "0:先攻 or 1:後攻" << std::endl;
  std::cout << ">> ";
  std::cin >> tmp;
  while (!std::cin.good() || (tmp != 0 && tmp != 1)) {
    std::cout << "もう一度入力してください。" << std::endl;
    std::cin.clear();
    std::cin.ignore(256, '\n');
    std::cout << ">> ";
    std::cin >> tmp;
  }
  player_color = (tmp == 0) ? OthelloState::kBlackTurn : OthelloState::kWhiteTurn;

  while (!state.isFinished()) {
    std::cout << "********************" << std::endl;
    if (state.getCurrentPlayerNum() == OthelloState::kBlackTurn) {
      std::cout << "黒の手番" << std::endl;
    } else {
      std::cout << "白の手番" << std::endl;
    }
    std::cout << " 黒: " << state.countDisksOf(OthelloState::kBlackTurn) << std::endl;
    std::cout << " 白: " << state.countDisksOf(OthelloState::kWhiteTurn) << std::endl;
    std::cout << "********************" << std::endl;

    state.print();
    coord action{};
    if (state.getCurrentPlayerNum() != player_color) {
      action = getPMCInput(state);
      // action = getMCTSInput(state);
    } else {
      action = getPlayerInput(state);
    }

    state = state.next(action);
  }

  if (state.getScore(OthelloState::kBlackTurn) == 1) {
    std::cout << "黒の勝ち" << std::endl;
  } else if (state.getScore(OthelloState::kWhiteTurn) == 1) {
    std::cout << "白の勝ち" << std::endl;
  } else {
    std::cout << "引き分け" << std::endl;
  }
}

int main(int argc, char* argv[]) {
  bool is_pvp{false};
  if (argc > 1) {
    is_pvp = strcmp(argv[1], "-p") == 0;
  }

  if (is_pvp) {
    pvp();
  } else {
    monte_carlo();
  }
}
