#include <string.h>
#include <iostream>

#include "../monte_carlo_tree_node.hpp"
#include "othello_state.hpp"

void pvp() {
  OthelloState state{};
  while (!state.isFinished()) {
    if (state.getCurTurn() == OthelloState::kBlackTurn) {
      std::cout << "黒の手番" << std::endl;
    } else {
      std::cout << "白の手番" << std::endl;
    }
    std::cout << std::endl;

    state.print();
    std::cout << std::endl;

    std::cout << "石を置く場所を指定してください。" << std::endl;
    OthelloState::coord action{};
    std::cout << "着手を入力してください。" << std::endl;
    std::cout << ">> ";
    std::cin >> action.first >> action.second;

    while (!std::cin.good() ||
        action.first < 0 || action.first >= 8 ||
        action.second < 0 || action.second >= 8 ||
        !state.isLegal(action)) {
      std::cout << "合法手ではありません。もう一度入力してください。" << std::endl;
      std::cin.clear();
      std::cin.ignore(256, '\n');
      std::cout << ">> ";
      std::cin >> action.first >> action.second;
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

void monte_carlo() {
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
    if (state.getCurTurn() == OthelloState::kBlackTurn) {
      std::cout << "黒の手番" << std::endl;
    } else {
      std::cout << "白の手番" << std::endl;
    }
    std::cout << " 黒: " << state.countDisksOf(OthelloState::kBlackTurn) << std::endl;
    std::cout << " 白: " << state.countDisksOf(OthelloState::kWhiteTurn) << std::endl;
    std::cout << "********************" << std::endl;

    state.print();
    OthelloState::coord action{};
    if (state.getCurTurn() != player_color) {
      int opponent_color{player_color == OthelloState::kBlackTurn ? OthelloState::kWhiteTurn : OthelloState::kBlackTurn};
      MonteCarloTreeNode<OthelloState, OthelloState::coord, 2> node{MonteCarloTreeNode<OthelloState, OthelloState::coord, 2>(state, opponent_color)};
      action = node.search();
    } else {
      std::cout << std::endl;

      std::cout << "石を置く場所を指定してください。" << std::endl;
      std::cout << "着手を入力してください。" << std::endl;
      std::cin >> action.first >> action.second;

      while (!std::cin.good() ||
             action.first < 0 || action.first >= 8 ||
             action.second < 0 || action.second >= 8 ||
             !state.isLegal(action)) {
        std::cout << "合法手ではありません。もう一度入力してください。" << std::endl;
        std::cin.clear();
        std::cin.ignore(256, '\n');
        std::cin >> action.first >> action.second;
      }
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
