#ifndef GAME_STATE_HPP_
#define GAME_STATE_HPP_

#include <vector>

template <typename GameAction>
class GameState {
 public:
  /* 受け取った手を適用して得られる状態を返す。 */
  virtual GameState next(GameAction const &action) = 0;

  /* 合法手の全体を返す。 */
  virtual std::vector<GameAction> legalActions() const = 0;

  /* ゲームが終了しているか？ */
  virtual bool is_finished() const = 0;

  /* 指定されたプレイヤ番号の現時点での得点を返す(囲碁将棋ならゲームが終わるまで常に0)。 */
  virtual int getScore(int player_num) const = 0;

  /* 最後の着手(この盤面に至ったときの着手)を返す。 */
  virtual GameAction getLastAction() const = 0;
};

#endif // GAME_STATE_HPP_
