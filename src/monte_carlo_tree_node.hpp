#ifndef MONTE_CARLO_TREE_NODE_HPP_
#define MONTE_CARLO_TREE_NODE_HPP_

#include <algorithm>
#include <functional>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

#include "xorshift64.hpp"

/* GameState: GameStateクラスを実装した型。 */
/* GameAction: ゲームの着手を表現する型。 */
template <class GameState, typename GameAction, int kNumberOfPlayers>
class MonteCarloTreeNode {
 public:
  /* このクラスをvectorで扱うために必要。 */
  MonteCarloTreeNode() : current_state_(), player_num_() {}

  MonteCarloTreeNode(const GameState& state, const int player_num, const GameAction& last_action, const unsigned int random_seed = 0, const float epsilon = 0.0, std::function<GameAction(const GameState&, XorShift64&)> selectForPlayout = randomAction)
      : current_state_(state), player_num_(player_num), last_action_(last_action), random_seed_(random_seed), random_engine_(random_seed_), selectForPlayout_(selectForPlayout), epsilon_(epsilon) {}

  /* 根用。クラスの外側から探索を指示されて最善手を返す。 */
  GameAction search() {
    int whole_play_cnt{};

    this->expand();

    /* 探索できない。 */
    if (this->children_.size() <= 0) {
      std::cerr << "子節点がありません。" << std::endl;
      std::terminate();
    }

    /* 手が1つしかないなら、それを出す。 */
    if (this->children_.size() == 1) {
      return this->children_.at(0).last_action_;
    }

    /* 探索。とりあえず、時間ではなく回数で探索に制限をかける。 */
    while (whole_play_cnt < MonteCarloTreeNode::kPlayoutLimit) {
      whole_play_cnt++;
      this->searchChild(whole_play_cnt);
    }

    /* [デバッグ] 各子節点の状態と評価値を出力する。 */
    if (MonteCarloTreeNode::kIsDebugMode) {
      for (const MonteCarloTreeNode<GameState, GameAction, kNumberOfPlayers>& child : this->children_) {
        std::cout << "********************" << std::endl;
        std::cout << "プレイヤ番号: " << player_num_ << std::endl;
        std::cout << "総プレイアウト回数: " << whole_play_cnt << std::endl;
        std::cout << "節点の通過回数: " << child.play_cnt_ << std::endl;
        std::cout << "得点和: ";
        for (const int s : sum_scores_) {
          std::cout << s << " ";
        }
        std::cout << std::endl;
        std::cout << "勝率: " << child.meanScore(player_num_) << std::endl;
        std::cout << "********************" << std::endl;
        child.current_state_.print();
        std::cout << "********************" << std::endl;
      }
    }

    /* 最善手を選んで返す。 */
    return this->selectChildWithBestMeanScore().last_action_;
  }

 private:
  static constexpr bool kIsDebugMode{false}; // デバッグ出力あり？
  static constexpr int kPlayoutLimit{1000};  // プレイアウト回数の制限。
  static constexpr int kExpandThreshold{3};  // 何回探索されたら節点を展開するか。
  static constexpr double kEvaluationMax{std::numeric_limits<double>::infinity()}; // 評価値の上限。

  GameState current_state_;  // 現在の局面情報。
  int player_num_;           // 自分のプレイヤ番号。
  GameAction last_action_{}; // この節点に遷移した際の行動。
  std::vector<MonteCarloTreeNode> children_{}; // 子節点(あり得る局面の集合)。
  int play_cnt_{};                             // この節点を探索した回数。
  std::array<double, kNumberOfPlayers> sum_scores_{}; // この局面を通るプレイアウトで得られた各プレイヤの総得点。勝1点負0点制なら勝利数と一致する。
  std::array<double, kNumberOfPlayers> sum_scores_squared_{}; // この局面を通るプレイアウトで得られた各プレイヤの得点の二乗値の総和。
  unsigned int random_seed_;
  XorShift64 random_engine_;
  std::function<GameAction(const GameState&, XorShift64&)> selectForPlayout_; // ロールアウトポリシー。
  float epsilon_{};

  /* 節点用。子節点を再帰的に掘り進め、各プレイヤの得点を逆伝播。 */
  std::array<double, kNumberOfPlayers> searchChild(int whole_play_cnt) {
    play_cnt_++;

    /* 既に勝敗がついていたら、結果を返す。 */
    if (this->current_state_.isFinished()) {
      std::array<double, kNumberOfPlayers> result{};
      for (int i = 0; i < kNumberOfPlayers; i++) {
        result.at(i) = this->current_state_.getScore(i);
      }
      /* 得点が負値になると二乗したとき良し悪しがわからなくなるので、得点の最小値を0にする。 */
      const double min_score{*std::min_element(result.begin(), result.end())};
      for (int i = 0; i < kNumberOfPlayers; i++) {
        result.at(i) -= min_score;
        sum_scores_.at(i) += result.at(i);
        sum_scores_squared_.at(i) += result.at(i) * result.at(i);
      }
      return result;
    }

    /* 子供がおらず、十分この節点を探索した場合は、展開する。 */
    if (this->children_.size() <= 0 &&
        this->play_cnt_ > MonteCarloTreeNode::kExpandThreshold) {
      this->expand();
    }

    /* 子供がいる場合は、選択して掘り進める。 */
    if (this->children_.size() > 0) {
      MonteCarloTreeNode<GameState, GameAction, kNumberOfPlayers>& child{this->selectChildToSearch(whole_play_cnt)};
      std::array<double, kNumberOfPlayers> result{child.searchChild(whole_play_cnt)};
      for (int i = 0; i < kNumberOfPlayers; i++) {
        sum_scores_.at(i) += result.at(i);
        sum_scores_squared_.at(i) += result.at(i) * result.at(i);
      }
      return result;
    }

    /* 子供がいない場合は、プレイアウトの結果を返す。 */
    std::array<double, kNumberOfPlayers> result{this->playout()};
    for (int i = 0; i < kNumberOfPlayers; i++) {
      sum_scores_.at(i) += result.at(i);
      sum_scores_squared_.at(i) += result.at(i) * result.at(i);
    }
    return result;
  }

  /* 子節点中で最も評価値の高いものを返す。 */
  MonteCarloTreeNode<GameState, GameAction, kNumberOfPlayers>& selectChildToSearch(int whole_play_cnt) {
    if (this->children_.size() <= 0) {
      std::cerr << "子節点がありません。" << std::endl;
      std::terminate();
    }

    return *std::max_element(
        this->children_.begin(), this->children_.end(),
        [whole_play_cnt, this](const MonteCarloTreeNode& a, const MonteCarloTreeNode& b) {
          return a.evaluate(whole_play_cnt, player_num_) < b.evaluate(whole_play_cnt, player_num_);
        });
  }

  /* 子節点中で最も勝率の高いものを返す。 */
  MonteCarloTreeNode<GameState, GameAction, kNumberOfPlayers>& selectChildWithBestMeanScore() {
    if (this->children_.size() <= 0) {
      std::cerr << "子節点がありません。" << std::endl;
      std::terminate();
    }

    return *std::max_element(
        this->children_.begin(), this->children_.end(),
        [this](const MonteCarloTreeNode& a, const MonteCarloTreeNode& b) {
          return a.meanScore(player_num_) < b.meanScore(player_num_);
        });
  }

  /* 可能な次局面すべてを子節点として追加。 */
  void expand() {
    std::vector<GameAction> actions{this->current_state_.legalActions()};
    this->children_.resize(actions.size());
    std::transform(actions.begin(), actions.end(), this->children_.begin(),
        [&](auto action) {
          GameState state{GameState(this->current_state_).next(action)};

          return MonteCarloTreeNode(state, state.getCurrentPlayerNum(), action, random_seed_, epsilon_, selectForPlayout_);
        });
  }

  /* プレイアウトを実施し、結果を返す。 */
  std::array<double, kNumberOfPlayers> playout() {
    GameState state{this->current_state_};

    while (!state.isFinished()) {
      state = state.next(epsilonGreedyAction(state));
    }

    std::array<double, kNumberOfPlayers> result{};
    for (int i = 0; i < kNumberOfPlayers; i++) {
      result.at(i) = state.getScore(i);
    }
    /* 得点が負値になると二乗したとき良し悪しがわからなくなるので、得点の最小値を0にする。 */
    const double min_score{*std::min_element(result.begin(), result.end())};
    std::transform(result.begin(), result.end(), result.begin(),
        [min_score](int score) { return score - min_score; });

    return result;
  }

  /* なんらかの方法でplayer_num目線での現在局面の評価値を計算して返す。 */
  double evaluate(int whole_play_cnt, int player_num) const {
    // return MonteCarloTreeNode::ucb1(whole_play_cnt, this->play_cnt_, this->sum_scores_.at(player_num));
    return MonteCarloTreeNode::ucb1Tuned(whole_play_cnt, this->play_cnt_, this->sum_scores_.at(player_num), this->sum_scores_squared_.at(player_num));
  }

  /* player_num目線での現在局面の平均得点を返す。勝ち点1負け点0のゲームなら勝率。 */
  double meanScore(int player_num) const {
    return (double)this->sum_scores_.at(player_num) / this->play_cnt_;
  }

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

  /* 与えられた局面に対してランダムな着手を選択。 */
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

  /* 確率kEpsilonでランダムな手を打つ。 */
  const GameAction epsilonGreedyAction(GameState& first_state) {
    std::bernoulli_distribution dist(epsilon_);

    if (dist(random_engine_)) {
      return randomAction(first_state, random_engine_);
    } else {
      return selectForPlayout_(first_state, random_engine_);
    }
  }
};

#endif  // MONTE_CARLO_TREE_NODE_HPP_
