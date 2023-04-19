#ifndef PRIMITIVE_MONTE_CARLO_ROOT_HPP_
#define PRIMITIVE_MONTE_CARLO_ROOT_HPP_

#include <cassert>

#include "primitive_monte_carlo_leaf.hpp"

template <class GameState, class GameObservation, class StateEstimator, typename GameAction, int kNumberOfPlayers>
class PrimitiveMonteCarloRoot {
 public:
  PrimitiveMonteCarloRoot(const GameObservation& observation, StateEstimator& estimator, const int player_num)
      : observation_(observation), player_num_(player_num), state_estimator_(estimator) {}

  GameAction search() {
    this->expand();

    /* 探索できない。 */
    assert(this->children_.size() > 0);

    /* 手が1つしかないなら、それを出す。 */
    if (this->children_.size() == 1) {
      return this->children_.at(0).getLastAction();
    }

    /* 評価。とりあえず、時間ではなく回数で評価回数に制限をかける。 */
    for (int whole_play_cnt = 0; whole_play_cnt < PrimitiveMonteCarloRoot::kPlayoutLimit * (int)this->children_.size(); whole_play_cnt++) {
      PrimitiveMonteCarloLeaf<GameState, GameAction, kNumberOfPlayers>& child{this->selectChildToSearch(whole_play_cnt)};
      child.playout();
    }

    /* 最善手を選んで返す。 */
    return this->selectChildWithBestMeanScore().getLastAction();
  }

 private:
  static constexpr int kPlayoutLimit{2000};  // プレイアウト回数の制限。

  GameObservation observation_; // 現在の局面情報。
  int player_num_;              // 自分のプレイヤ番号。
  StateEstimator state_estimator_;
  std::vector<PrimitiveMonteCarloLeaf<GameState, GameAction, kNumberOfPlayers>> children_{}; // 子節点(あり得る局面の集合)。

  /* 可能な次局面すべてを子節点として追加。 */
  void expand() {
    /* 現在状態を推定。 */
    const GameState current_state{this->state_estimator_.estimate(this->observation_)};

    /* 子節点を作る。 */
    const std::vector<GameAction> actions{this->observation_.legal_actions_};
    this->children_.resize(actions.size());
    std::transform(actions.begin(), actions.end(), this->children_.begin(),
        [&](auto action) {
          GameState state{GameState(current_state).next(action)};
          return PrimitiveMonteCarloLeaf<GameState, GameAction, kNumberOfPlayers>(state, action);
        });
  }

  /* 子節点中で最も評価値の高いものを返す。 */
  PrimitiveMonteCarloLeaf<GameState, GameAction, kNumberOfPlayers>& selectChildToSearch(int whole_play_cnt) {
    assert(this->children_.size() > 0);

    return *std::max_element(
        this->children_.begin(), this->children_.end(),
        [whole_play_cnt, this](const auto& a, const auto& b) {
          return a.evaluate(whole_play_cnt, player_num_) < b.evaluate(whole_play_cnt, player_num_);
        });
  }

  /* 子節点中で最も勝率の高いものを返す。 */
  PrimitiveMonteCarloLeaf<GameState, GameAction, kNumberOfPlayers>& selectChildWithBestMeanScore() {
    assert(this->children_.size() > 0);

    return *std::max_element(
        this->children_.begin(), this->children_.end(),
        [this](const auto& a, const auto& b) {
          return a.meanScore(player_num_) < b.meanScore(player_num_);
        });
  }
};

#endif // PRIMITIVE_MONTE_CARLO_ROOT_HPP_
