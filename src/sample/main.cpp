#include <random>
#include <string.h>
#include <iostream>

#include "../monte_carlo_tree_node.hpp"
#include "othello_state.hpp"

OthelloState::coord getInputCoord(const OthelloState& state) {
  std::cout << "石を置く場所を指定してください。" << std::endl;
  std::cout << "着手を入力してください。" << std::endl;
  std::cout << ">> ";
  std::string input{};
  std::cin >> input;
  OthelloState::coord action{OthelloState::str2Coord(input)};

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

void pvp() {
  OthelloState state{};
  while (!state.isFinished()) {
    if (state.getMyPlayerNum() == OthelloState::kBlackTurn) {
      std::cout << "黒の手番" << std::endl;
    } else {
      std::cout << "白の手番" << std::endl;
    }
    std::cout << std::endl;

    state.print();
    std::cout << std::endl;
    state = state.next(getInputCoord(state));
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

  /* 乱数のシード生成器。 */
  std::random_device seed_gen;

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
    if (state.getMyPlayerNum() == OthelloState::kBlackTurn) {
      std::cout << "黒の手番" << std::endl;
    } else {
      std::cout << "白の手番" << std::endl;
    }
    std::cout << " 黒: " << state.countDisksOf(OthelloState::kBlackTurn) << std::endl;
    std::cout << " 白: " << state.countDisksOf(OthelloState::kWhiteTurn) << std::endl;
    std::cout << "********************" << std::endl;

    state.print();
    OthelloState::coord action{};
    if (state.getMyPlayerNum() != player_color) {
      int opponent_color{player_color == OthelloState::kBlackTurn ? OthelloState::kWhiteTurn : OthelloState::kBlackTurn};
      MonteCarloTreeNode<OthelloState, OthelloState::coord, 2> node{MonteCarloTreeNode<OthelloState, OthelloState::coord, 2>(state, opponent_color, seed_gen())};
      action = node.search();
    } else {
      action = getInputCoord(state);
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
