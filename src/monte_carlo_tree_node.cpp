#include "monte_carlo_tree_node.hpp"

template <class GameState, typename GameAction>
GameAction MonteCarloTreeNode<GameState, GameAction>::search() {
  int whole_play_cnt = 0;

  this->expand();

  /* 探索できない。 */
  if (this->children_.size() <= 0) {
    std::cerr << "子節点がありません。" << std::endl;
    std::terminate();
  }

  /* 探索。とりあえず、時間ではなく回数で探索に制限をかける。 */
  while (whole_play_cnt < MonteCarloTreeNode::kPlayoutLimit) {
    whole_play_cnt++;
    this->search_child(whole_play_cnt);
  }

  return this->select(whole_play_cnt).current_state_.getLastAction();
}

template <class GameState, typename GameAction>
MonteCarloTreeNode<GameState, GameAction>& MonteCarloTreeNode<GameState, GameAction>::select(
    int whole_play_cnt) {
  if (this->children_.size() <= 0) {
    std::cerr << "子節点がありません。" << std::endl;
    std::terminate();
  }

  auto best_child = std::max_element(
      this->children_.begin(), this->children_.end(),
      [=](const MonteCarloTreeNode& a, const MonteCarloTreeNode& b) {
        return a.evaluate(whole_play_cnt) < b.evaluate(whole_play_cnt);
      });

  return *best_child;
}

template <class GameState, typename GameAction>
void MonteCarloTreeNode<GameState, GameAction>::expand() {
  std::vector<GameAction> actions = this->current_state_.legalActions();
  std::transform(actions.begin(), actions.end(), this->children_.begin(),
                 [&](auto action) {
                   GameState state =
                       GameState(this->current_state_).next(action);
                   return MonteCarloTreeNode(state, this->player_num_);
                 });
}

template <class GameState, typename GameAction>
int MonteCarloTreeNode<GameState, GameAction>::search_child(int whole_play_cnt) {
  this->play_cnt_++;

  /* 既に勝敗がついていたら、結果を返す。 */
  if (this->current_state_.is_finished()) {
    return this->current_state_.getScore(this->player_num_);
  }

  /* 子供がおらず、十分この節点を探索した場合は、展開する。 */
  if (this->children_.size() <= 0 && this->play_cnt_ > MonteCarloTreeNode::kExpandThreshold) {
    this->expand();
  }

  /* 子供がいる場合は、選択して掘り進める。 */
  if (this->children_.size() > 0) {
    MonteCarloTreeNode<GameState, GameAction>& child = this->select(whole_play_cnt);
    return child.search_child(whole_play_cnt);
  }

  /* プレイアウトの結果を返す。 */
  return this->playout();
}

template <class GameState, typename GameAction>
int MonteCarloTreeNode<GameState, GameAction>::playout() {
  GameState state = this->current_state_;

  while (!state.is_finished()) {
    state = state.next(MonteCarloTreeNode::randomAction(state));
  }

  return state.getScore(this->player_num_);
}
