#ifndef OTHELLO_STATE_HPP_
#define OTHELLO_STATE_HPP_

#include <cassert>
#include <array>
#include <iostream>
#include <vector>

#include "othello_types.hpp"
#include "othello_observation.hpp"

class OthelloState {
 public:
  static constexpr int kBlackTurn{0};
  static constexpr int kWhiteTurn{1};

  /* ゲーム初期化用。 */
  OthelloState() = default;

  /* OthelloObservationから組み立てる用。 */
  OthelloState(const bitboard& black_board, const bitboard& white_board, const int cur_turn)
      : black_board_(black_board), white_board_(white_board), cur_turn_(cur_turn) {}

  /* 受け取った手を適用して得られる状態を返す。 */
  OthelloState next(const coord& action) const;

  /* 合法手の全体を返す。 */
  std::vector<coord> legalActions() const;

  /* ゲームが終了しているか？ */
  bool isFinished() const;

  /* 合法手か(外向け)。 */
  bool isLegal(const coord put) const {
    return this->isLegal(coord2Bit(put));
  }

  /* 座標をbit表現に変換。 */
  /* 指定されたプレイヤ番号の現時点での得点を返す。 */
  int getScore(const int player_num) const;

  /* 現在どちらの手番か。 */
  int getCurrentPlayerNum() const { return this->cur_turn_; }

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

  OthelloObservation getObservation() const {
    return OthelloObservation{
      this->black_board_,
      this->white_board_,
      this->cur_turn_,
      this->legalActions()
    };
  }

  /* 盤面を文字列に変換。 */
  std::string board2String() const;

  /* 盤面を出力。 */
  void print() const { std::cout << board2String(); }

  static bitboard coord2Bit(const coord xy) {
    return OthelloState::kSquare.at(xy.first + 8 * xy.second);
  }

  static coord str2Coord(std::string str);

  static std::string coord2Str(coord c);

 private:
  /* マスの表現。A1, B1, ..., H1, A2, ..., H8 の順。 */
  static const std::array<bitboard, 64> kSquare;

  bitboard black_board_{0x00'00'00'08'10'00'00'00};
  bitboard white_board_{0x00'00'00'10'08'00'00'00};
  int cur_turn_{OthelloState::kBlackTurn};

  /* 合法手か。 */
  bool isLegal(const bitboard put) const {
    return (put & this->legalBoard()) == put;
  }

  /* 置ける場所の一覧をbit表現で返す。 */
  bitboard legalBoard() const;

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
