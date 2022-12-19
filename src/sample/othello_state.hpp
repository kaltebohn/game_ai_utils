#ifndef OTHELLO_STATE_HPP_
#define OTHELLO_STATE_HPP_

#include <array>
#include <iostream>
#include <vector>

class OthelloState {
 public:
  using bitboard = uint64_t;
  using coord = std::pair<int, int>;  // A4は{0, 3} で表現。

  static constexpr int kBlackTurn{0};
  static constexpr int kWhiteTurn{1};

  /* 受け取った手を適用して得られる状態を返す。 */
  OthelloState next(const coord& action) const;

  /* 合法手の全体を返す。 */
  std::vector<coord> legalActions() const;

  /* ゲームが終了しているか？ */
  bool isFinished() const;

  /* 合法手か(外向け)。 */
  bool isLegal(const OthelloState::coord put) const {
    return this->isLegal(coord2Bit(put));
  }

  /* 座標をbit表現に変換。 */
  static bitboard coord2Bit(const coord xy) {
    return OthelloState::kSquare.at(xy.first + 8 * xy.second);
  }

  /* 指定されたプレイヤ番号の現時点での得点を返す。 */
  int getScore(const int player_num) const;

  /* 最後の着手(この盤面に至ったときの着手)を返す。 */
  coord getLastAction() const { return this->last_action_; }

  /* 現在どちらの手番か。 */
  int getMyPlayerNum() const { return this->cur_turn_; }

  /* 盤面を文字列に変換。 */
  std::string board2String() const;

  /* 盤面を出力。 */
  void print() const { std::cout << board2String(); }

  int countDisksOf(int player_num) const {
    switch (player_num) {
      case OthelloState::kBlackTurn:
        return this->count(this->black_board_);
      case OthelloState::kWhiteTurn:
        return this->count(this->white_board_);
      default:
        return 0;
    }
  }

 private:
  /* マスの表現。A1, B1, ..., H1, A2, ..., H8 の順。 */
  static const std::array<bitboard, 64> kSquare;

  bitboard black_board_{0x00'00'00'08'10'00'00'00};
  bitboard white_board_{0x00'00'00'10'08'00'00'00};
  int cur_turn_{OthelloState::kBlackTurn};
  coord last_action_{};

  /* 合法手か。 */
  bool isLegal(const bitboard put) const {
    return (put & this->legalBoard()) == put;
  }

  /* 置ける場所の一覧をbit表現で返す。 */
  bitboard legalBoard() const;

  /* 着手のbit表現を受け取り、次の局面のbit表現を返す。 */
  bitboard nextBoard(const bitboard put) const;

  /* squareをdirectionの向きに移す。 */
  bitboard nextSquare(const bitboard square, int direction) const;

  /* パスかどうか。 */
  bool isPass() const;

  /* bitboardの1を数える。 */
  static int count(const bitboard src) {
    bitboard tmp{src};
    tmp = (tmp & 0x5555555555555555) + (tmp >> 1 & 0x5555555555555555);   // 2bits区切でビット数を数える。
    tmp = (tmp & 0x3333333333333333) + (tmp >> 2 & 0x3333333333333333);   // 4bits区切。
    tmp = (tmp & 0x0f0f0f0f0f0f0f0f) + (tmp >> 4 & 0x0f0f0f0f0f0f0f0f);   // 8bits。
    tmp = (tmp & 0x00ff00ff00ff00ff) + (tmp >> 8 & 0x00ff00ff00ff00ff);   // 16bits。
    tmp = (tmp & 0x0000ffff0000ffff) + (tmp >> 16 & 0x0000ffff0000ffff);  // 32bits。
    return (tmp & 0x00000000ffffffff) + (tmp >> 32 & 0x00000000ffffffff); // 64bits。
  }

  /* デバッグ用。状態クラスの出力。 */
  friend std::ostream& operator<<(std::ostream& os, const OthelloState& src);
};

#endif  // OTHELLO_STATE_HPP_
