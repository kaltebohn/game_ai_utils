#ifndef MONTE_CARLO_TREE_NODE_HPP_
#define MONTE_CARLO_TREE_NODE_HPP_

#include <algorithm>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

/* GameState: GameStateクラスを実装した型。 */
/* GameAction: ゲームの着手を表現する型。 */
template <class GameState, typename GameAction>
class MonteCarloTreeNode {
 public:
  MonteCarloTreeNode(const GameState& state, const int player_num)
      : player_num_(player_num), current_state_(state) {}

  /* 根用。クラスの外側から探索を指示されて最善手を返す。 */
  GameAction search();

 private:
  static constexpr int kPlayoutLimit = 100;    // プレイアウト回数の制限。
  static constexpr int kExpandThreshold = 3;   // 何回探索されたら節点を展開するか。
  static const double kEvaluationMax =
      std::numeric_limits<double>::infinity(); // 評価値の上限。
  GameState current_state_;                    // 現在の局面情報。
  std::vector<MonteCarloTreeNode> children_;   // 子節点(あり得る局面の集合)。
  int player_num_;                             // 自分のプレイヤ番号。
  int play_cnt_;                               // この節点を探索した回数。
  int sum_score_;                              // この局面を通るプレイアウトで得られた得点の総数。勝1点負0点制なら勝利数と一致する。

  /* 節点用。子節点を再帰的に掘り進め、勝利数を逆伝播。 */
  int search_child(int whole_play_cnt);

  /* 子節点中で最も評価値の高いものを返す。 */
  MonteCarloTreeNode& select(int whole_play_cnt);

  /* 可能な次局面すべてを子節点として追加。 */
  void expand();

  /* プレイアウトを実施し、結果を返す。 */
  int playout();

  /* なんらかの方法で現在局面の評価値を計算して返す。 */
  double evaluate(int whole_play_cnt) const {
    return MonteCarloTreeNode::ucb1(whole_play_cnt, this->play_cnt_,
                                    this->sum_score_);
  }

  /* ucb1値を返す。 */
  /* 得点制ゲームに対応するため、勝ち数の代わりに得点を用いている。オセロや将棋では勝ち1、負け0にすればよい。
   */
  static double ucb1(int whole_play_cnt, int play_cnt, int score) {
    if (play_cnt <= 0) {
      return kEvaluationMax;
    } else {
      return (double)score / play_cnt +
             std::sqrt(2.0 * std::log2(whole_play_cnt) / play_cnt);
    }
  }

  /* 与えられた局面に対してランダムな着手を選択。 */
  static const GameAction randomAction(GameState first_state) {
    std::random_device seed_gen;
    std::default_random_engine rand_engine(seed_gen());

    std::vector<GameAction> actions = first_state.legalActions();
    std::uniform_int_distribution<int> dist(0, actions.size() - 1);

    return actions.at(dist(rand_engine));
  }
};

#endif  // MONTE_CARLO_TREE_NODE_HPP_
