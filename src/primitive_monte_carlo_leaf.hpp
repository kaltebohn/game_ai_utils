#ifndef MONTE_CARLO_TREE_LEAF_HPP_
#define MONTE_CARLO_TREE_LEAF_HPP_

#include <functional>
#include <random>

#include <iostream>

#include "xorshift64.hpp"

template <class GameState, typename GameAction, int kNumberOfPlayers>
class PrimitiveMonteCarloLeaf {
 public:
  /* このクラスをvectorで扱うために必要。 */
  PrimitiveMonteCarloLeaf() : last_action_() {}

  PrimitiveMonteCarloLeaf(const GameAction action)
      : last_action_(action) {}

  /* この葉節点から見て現在の状態からプレイアウトを実施し、結果を返す。 */
  void playout(const GameState& current_state, std::function<GameAction(const GameState&, XorShift64&)> playout_policy) {
    this->play_cnt_++;

    /* 乱数生成器。 */
    std::random_device seed_gen;
    XorShift64 random_engine{seed_gen()};

    /* プレイアウト。 */
    GameState state{current_state};
    while (!state.isFinished()) {
      state = state.next(playout_policy(state, random_engine));
    }

    /* 評価。 */
    std::array<double, kNumberOfPlayers> result{};
    for (int i = 0; i < kNumberOfPlayers; i++) {
      result.at(i) = state.getScore(i);
    }

    /* 得点をMin-Max正規化。 */
    const double min_score{*std::min_element(result.begin(), result.end())};
    const double max_score{*std::max_element(result.begin(), result.end())};
    if (min_score == max_score) { return; } // もし全員0点(UNOで全員が0のカードを持っている場合など)なら結果反映の必要なし。
    std::transform(result.begin(), result.end(), result.begin(),
        [max_score, min_score](int score) { return (score - min_score) / (max_score - min_score); });

    /* 結果を反映。 */
    for (int i = 0; i < kNumberOfPlayers; i++) {
      sum_scores_.at(i) += result.at(i);
      sum_scores_squared_.at(i) += result.at(i) * result.at(i);
    }
  }

  /* なんらかの方法でplayer_num目線での現在局面の評価値を計算して返す。 */
  double evaluate(int whole_play_cnt, int player_num) const {
    // return PrimitiveMonteCarloLeaf::ucb1(whole_play_cnt, this->play_cnt_, this->sum_scores_.at(player_num));
    return PrimitiveMonteCarloLeaf::ucb1Tuned(whole_play_cnt, this->play_cnt_, this->sum_scores_.at(player_num), this->sum_scores_squared_.at(player_num));
  }

  /* player_num目線での現在局面の平均得点を返す。勝ち点1負け点0のゲームなら勝率。 */
  double meanScore(int player_num) const {
    return (double)this->sum_scores_.at(player_num) / this->play_cnt_;
  }

  GameAction getLastAction() const { return this->last_action_; }

 private:
  static constexpr double kEvaluationMax{std::numeric_limits<double>::infinity()}; // 評価値の上限。

  GameAction last_action_;
  int play_cnt_{};
  std::array<double, kNumberOfPlayers> sum_scores_{}; // この局面を通るプレイアウトで得られた各プレイヤの総得点。勝1点負0点制なら勝利数と一致する。
  std::array<double, kNumberOfPlayers> sum_scores_squared_{}; // この局面を通るプレイアウトで得られた各プレイヤの得点の二乗値の総和。

  /* ucb1値を返す。 */
  /* 得点制ゲームに対応するため、勝ち数の代わりに得点を用いている。オセロや将棋では勝ち1、負け0にすればよい。 */
  static double ucb1(int whole_play_cnt, int play_cnt, int score) {
    return (play_cnt <= 0) ? kEvaluationMax : (double)score / play_cnt + std::sqrt(2.0 * std::log2(whole_play_cnt) / play_cnt);
  }

  /* ucb1-tuned値を返す。 */
  static double ucb1Tuned(int whole_play_cnt, int play_cnt, int score, int score_squared) {
    const double mean = (double)score / play_cnt;
    const double variance = score_squared - mean * mean;
    const double v = variance + std::sqrt(2.0 * std::log2(whole_play_cnt) / play_cnt);
    return (play_cnt <= 0) ? kEvaluationMax : (double)score / play_cnt + std::sqrt(std::log2(whole_play_cnt) / play_cnt * std::min(0.25, v));
  }
};

#endif // MONTE_CARLO_TREE_LEAF_HPP_
