#ifndef PRIMITIVE_MONTE_CARLO_ROOT_HPP_
#define PRIMITIVE_MONTE_CARLO_ROOT_HPP_

#include <cassert>

#include "primitive_monte_carlo_leaf.hpp"

template <class GameState, class GameObservation, class StateEstimator, typename GameAction, int kNumberOfPlayers>
class PrimitiveMonteCarloRoot {
 public:
  PrimitiveMonteCarloRoot(const GameObservation& observation, StateEstimator& estimator, const int player_num)
      : observation_(observation), player_num_(player_num), state_estimator_(estimator) {}

  GameAction search(std::function<GameAction(const GameState&, XorShift64&)> playout_policy = randomAction) {
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
      const GameState current_state{this->state_estimator_.estimate(this->observation_)}; // 現在状態を推定。
      child.playout(current_state.next(child.getLastAction()), playout_policy);
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

  static GameAction randomAction(const GameState& first_state, XorShift64& random_engine) {
    std::vector<GameAction> actions{first_state.legalActions()};
    if (actions.size() == 1) { return actions.at(0); } // 一手しかないなら、それを出す。

    // デバッグ。7bdc5ad時点で稀にエラーが発生するのでここで出力させる。
    if (actions.size() == 0) {
      std::cout << "合法手が存在しないエラー。" << std::endl;
      std::cout << first_state;
    }

    std::uniform_int_distribution<int> dist(0, actions.size() - 1);
    return actions.at(dist(random_engine));
  }

  /* 可能な次局面すべてを子節点として追加。 */
  void expand() {
    /* 子節点を作る。不完全情報ゲームで探索毎に状態を推定する場合のために、葉には状態を持たせない。 */
    const std::vector<GameAction> actions{this->observation_.legal_actions_};
    this->children_.resize(actions.size());
    std::transform(actions.begin(), actions.end(), this->children_.begin(),
        [&](auto action) {
          return PrimitiveMonteCarloLeaf<GameState, GameAction, kNumberOfPlayers>(action);
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
