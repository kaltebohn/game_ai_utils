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
  MonteCarloTreeNode() : current_state_(), player_num_() {}

  MonteCarloTreeNode(const GameState& state, const int player_num)
      : current_state_(state), player_num_(player_num) {}

  /* 根用。クラスの外側から探索を指示されて最善手を返す。 */
  GameAction search() {
    int whole_play_cnt{};

    this->expand();

    /* 探索できない。 */
    if (this->children_.size() <= 0) {
      std::cerr << "子節点がありません。" << std::endl;
      std::terminate();
    }

    /* 探索。とりあえず、時間ではなく回数で探索に制限をかける。 */
    while (whole_play_cnt < MonteCarloTreeNode::kPlayoutLimit) {
      whole_play_cnt++;
      this->searchChild(whole_play_cnt);
    }

    /* [デバッグ] 各子節点の状態と評価値を出力する。 */
    if (MonteCarloTreeNode::kIsDebug) {
      for (const MonteCarloTreeNode<GameState, GameAction>& child : this->children_) {
        std::cout << "********************" << std::endl;
        std::cout << "総プレイアウト回数: " << whole_play_cnt << std::endl;
        std::cout << "節点の通過回数: " << child.play_cnt_ << std::endl;
        std::cout << "得点和: " << child.sum_score_ << std::endl;
        std::cout << "勝率: " << child.meanScore() << std::endl;
        std::cout << "********************" << std::endl;
        child.current_state_.print();
        std::cout << "********************" << std::endl;
      }
    }

    /* 最善手を選んで返す。 */
    return this->selectChildWithBestMeanScore().current_state_.getLastAction();
  }

 private:
  static constexpr bool kIsDebug{false};    // デバッグ出力あり？
  static constexpr int kPlayoutLimit{1000}; // プレイアウト回数の制限。
  static constexpr int kExpandThreshold{3}; // 何回探索されたら節点を展開するか。
  static constexpr double kEvaluationMax{std::numeric_limits<double>::infinity()};  // 評価値の上限。
  GameState current_state_;                     // 現在の局面情報。
  std::vector<MonteCarloTreeNode> children_;  // 子節点(あり得る局面の集合)。
  int player_num_;                            // 自分のプレイヤ番号。
  int play_cnt_{};   // この節点を探索した回数。
  int sum_score_{};  // この局面を通るプレイアウトで得られた得点の総数。勝1点負0点制なら勝利数と一致する。

  /* 節点用。子節点を再帰的に掘り進め、勝利数を逆伝播。 */
  int searchChild(int whole_play_cnt) {
    this->play_cnt_++;

    /* 既に勝敗がついていたら、結果を返す。 */
    if (this->current_state_.isFinished()) {
      this->sum_score_ += this->current_state_.getScore(this->player_num_);
      return this->current_state_.getScore(this->player_num_);
    }

    /* 子供がおらず、十分この節点を探索した場合は、展開する。 */
    if (this->children_.size() <= 0 &&
        this->play_cnt_ > MonteCarloTreeNode::kExpandThreshold) {
      this->expand();
    }

    /* 子供がいる場合は、選択して掘り進める。 */
    if (this->children_.size() > 0) {
      MonteCarloTreeNode<GameState, GameAction>& child =
          this->selectChildToSearch(whole_play_cnt);
      int result{child.searchChild(whole_play_cnt)};
      this->sum_score_ += result;
      return result;
    }

    /* 子供がいない場合は、プレイアウトの結果を返す。 */
    int result{this->playout()};
    this->sum_score_ += result;
    return result;
  }

  /* 子節点中で最も評価値の高いものを返す。 */
  MonteCarloTreeNode<GameState, GameAction>& selectChildToSearch(
      int whole_play_cnt) {
    if (this->children_.size() <= 0) {
      std::cerr << "子節点がありません。" << std::endl;
      std::terminate();
    }

    return *std::max_element(
        this->children_.begin(), this->children_.end(),
        [=](const MonteCarloTreeNode& a, const MonteCarloTreeNode& b) {
          return a.evaluate(whole_play_cnt) < b.evaluate(whole_play_cnt);
        });
  }

  /* 子節点中で最も勝率の高いものを返す。 */
  MonteCarloTreeNode<GameState, GameAction>& selectChildWithBestMeanScore() {
    if (this->children_.size() <= 0) {
      std::cerr << "子節点がありません。" << std::endl;
      std::terminate();
    }

    return *std::max_element(
        this->children_.begin(), this->children_.end(),
        [=](const MonteCarloTreeNode& a, const MonteCarloTreeNode& b) {
          return a.meanScore() < b.meanScore();
        });
  }

  /* 可能な次局面すべてを子節点として追加。 */
  void expand() {
    std::vector<GameAction> actions{this->current_state_.legalActions()};
    this->children_.resize(actions.size());
    std::transform(actions.begin(), actions.end(), this->children_.begin(),
                   [&](auto action) {
                     GameState state =
                         GameState(this->current_state_).next(action);
                     return MonteCarloTreeNode(state, this->player_num_);
                   });
  }

  /* プレイアウトを実施し、結果を返す。 */
  int playout() {
    GameState state{this->current_state_};

    while (!state.isFinished()) {
      state = state.next(MonteCarloTreeNode::randomAction(state));
    }

    return state.getScore(this->player_num_);
  }

  /* なんらかの方法で現在局面の評価値を計算して返す。 */
  double evaluate(int whole_play_cnt) const {
    return MonteCarloTreeNode::ucb1(whole_play_cnt, this->play_cnt_,
                                    this->sum_score_);
  }

  /* 現在局面の平均得点を返す。勝ち点1負け点0のゲームなら勝率。 */
  double meanScore() const {
    return (double)this->sum_score_ / this->play_cnt_;
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

    std::vector<GameAction> actions{first_state.legalActions()};
    std::uniform_int_distribution<int> dist(0, actions.size() - 1);

    return actions.at(dist(rand_engine));
  }
};

#endif  // MONTE_CARLO_TREE_NODE_HPP_
